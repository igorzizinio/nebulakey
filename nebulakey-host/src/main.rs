use nebulakey_host::{PlaybackStatus, create_manager};
use serialport::{SerialPortType, available_ports};
use std::io::Write;
use std::time::{Duration, Instant};

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

    // Create media manager
    let manager = match create_manager().await {
        Ok(m) => m,
        Err(e) => {
            println!("Failed to create media manager: {}", e);
            return;
        }
    };

    // Get current session
    let session = match manager.get_current_session() {
        Ok(s) => s,
        Err(e) => {
            println!("Failed to get current session: {}", e);
            return;
        }
    };

    // Find the RP2040 (VID 0x2E8A)
    let ports = available_ports().expect("Failed to list serial ports");
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

    // track playback state
    let mut last_system_ticks: i64 = 0;
    let mut end_ticks: i64 = 0;
    let mut last_sync = Instant::now();
    let mut last_title = String::new();
    let mut last_status = PlaybackStatus::Stopped;

    loop {
        // Get media properties
        let media_props = match session.get_media_properties().await {
            Ok(p) => p,
            Err(e) => {
                println!("Failed to get media properties: {}", e);
                break;
            }
        };

        // Get timeline properties
        let timeline = match session.get_timeline_properties() {
            Ok(t) => t,
            Err(e) => {
                println!("Failed to get timeline properties: {}", e);
                break;
            }
        };

        // Get playback info
        let playback = match session.get_playback_status() {
            Ok(p) => p,
            Err(e) => {
                println!("Failed to get playback info: {}", e);
                break;
            }
        };

        let status = playback.status;
        let title = &media_props.title;

        port.write_all(format!("TRACK: {}\n", title).as_bytes())
            .expect("Failed to write to serial port");

        let current_position = timeline.position;
        let current_end = timeline.end_time;

        let system_changed = current_position != last_system_ticks
            || current_end != end_ticks
            || title != &last_title
            || status != last_status;

        if system_changed {
            last_system_ticks = current_position;
            end_ticks = current_end;
            last_sync = Instant::now();
            last_title = title.to_string();
            last_status = status;
        }

        let mut estimated_ticks = last_system_ticks;
        if status == PlaybackStatus::Playing {
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
