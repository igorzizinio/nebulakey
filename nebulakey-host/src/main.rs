use serialport::{SerialPortType, available_ports};
use std::io::Write;
use std::time::{Duration, Instant};
use windows::Foundation::TimeSpan;
use windows::Media::Control::*;

fn ticks_to_seconds(ticks: i64) -> u64 {
    if ticks <= 0 {
        return 0;
    }
    (ticks as u64) / 10_000_000
}

fn elapsed_to_ticks(elapsed: Duration) -> i64 {
    (elapsed.as_secs_f64() * 10_000_000.0) as i64
}

#[tokio::main]
async fn main() {
    println!("=== NebulaKey Host ===");

    // find the RP2040 (VID 0x2E8A)
    let ports = available_ports().expect("Failed to list serial ports");
    let op_request = GlobalSystemMediaTransportControlsSessionManager::RequestAsync();

    let manager = op_request
        .expect("failed to request media transpost manager")
        .await
        .expect("failed on await");

    let rp_port = ports.into_iter().find(|port| {
        if let SerialPortType::UsbPort(info) = &port.port_type {
            info.vid == 0x2E8A
        } else {
            false
        }
    });

    let port_info = match rp_port {
        Some(port) => port,
        None => {
            println!("NebulaKey not found.");
            return;
        }
    };

    println!("Found NebulaKey at: {}", port_info.port_name);

    let mut port = serialport::new(port_info.port_name, 115_200)
        .timeout(Duration::from_millis(200))
        .open()
        .expect("Failed to open serial port");

    let session = manager
        .GetCurrentSession()
        .expect("cant get current media session");

    // eveery second send the current track playing
    let mut last_system_ticks: i64 = 0;
    let mut end_ticks: i64 = 0;
    let mut last_sync = Instant::now();
    let mut last_title = String::new();
    let mut last_status = GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped;

    loop {
        let media_props = session
            .TryGetMediaPropertiesAsync()
            .expect("could not get media props")
            .await
            .expect("failed on await");

        let timeline = session
            .GetTimelineProperties()
            .expect("could not get timeline props");

        let playback = session
            .GetPlaybackInfo()
            .expect("could not get playback info");
        let status = playback
            .PlaybackStatus()
            .unwrap_or(GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped);

        port.write_all(format!("TRACK: {}\n", media_props.Title().unwrap_or_default()).as_bytes())
            .expect("Failed to write to serial port");

        let current_position = timeline.Position().unwrap_or(TimeSpan::default()).Duration;
        let current_end = timeline.EndTime().unwrap_or(TimeSpan::default()).Duration;
        let title = media_props.Title().unwrap_or_default();

        let system_changed = current_position != last_system_ticks
            || current_end != end_ticks
            || title != last_title
            || status != last_status;

        if system_changed {
            last_system_ticks = current_position;
            end_ticks = current_end;
            last_sync = Instant::now();
            last_title = title.to_string();
            last_status = status;
        }

        let mut estimated_ticks = last_system_ticks;
        if status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing {
            estimated_ticks =
                last_system_ticks.saturating_add(elapsed_to_ticks(last_sync.elapsed()));
            if end_ticks > 0 && estimated_ticks > end_ticks {
                estimated_ticks = end_ticks;
            }
        }

        let position_seconds = ticks_to_seconds(estimated_ticks);
        let end_seconds = ticks_to_seconds(end_ticks);

        println!("Track: {}", title);
        println!("Timeline: {}/{}", position_seconds, end_seconds);

        port.write_all(format!("TIMELINE: {}/{}\n", position_seconds, end_seconds).as_bytes())
            .expect("Failed to write to serial port");

        tokio::time::sleep(Duration::from_secs(1)).await;
    }
}
