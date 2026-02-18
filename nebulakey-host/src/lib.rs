#[cfg(target_os = "windows")]
use windows::Media::Control::*;

#[cfg(target_os = "linux")]
use mpris::PlayerFinder;

/// Platform-independent playback status enum
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PlaybackStatus {
    Playing,
    Paused,
    Stopped,
}

/// Platform-independent media properties
#[derive(Debug, Clone)]
pub struct MediaProperties {
    pub title: String,
}

/// Platform-independent timeline properties (in 100-nanosecond ticks)
#[derive(Debug, Clone)]
pub struct TimelineProperties {
    pub position: i64,
    pub end_time: i64,
}

/// Platform-independent playback info
#[derive(Debug, Clone)]
pub struct PlaybackInfo {
    pub status: PlaybackStatus,
}

/// Generic session manager
pub struct Manager {
    #[cfg(target_os = "windows")]
    inner: GlobalSystemMediaTransportControlsSessionManager,
}

/// Generic media session
pub struct Session {
    #[cfg(target_os = "windows")]
    inner: GlobalSystemMediaTransportControlsSession,

    #[cfg(target_os = "linux")]
    player: mpris::Player,
}

/// Create a new session manager
#[cfg(target_os = "windows")]
pub async fn create_manager() -> Result<Manager, String> {
    let op_request = GlobalSystemMediaTransportControlsSessionManager::RequestAsync();

    let manager = op_request
        .map_err(|_| "Failed to request media transport manager".to_string())?
        .await
        .map_err(|_| "Failed to await manager request".to_string())?;

    Ok(Manager { inner: manager })
}

#[cfg(target_os = "linux")]
pub async fn create_manager() -> Result<Manager, String> {
    Ok(Manager {})
}

/// macOS stub (placeholder for future implementation)
#[cfg(target_os = "macos")]
pub async fn create_manager() -> Result<Manager, String> {
    Err("Media control not yet implemented for this platform".to_string())
}

impl Manager {
    /// Get the current active media session
    #[cfg(target_os = "windows")]
    pub fn get_current_session(&self) -> Result<Session, String> {
        let session = self
            .inner
            .GetCurrentSession()
            .map_err(|_| "Failed to get current media session".to_string())?;

        Ok(Session { inner: session })
    }

    #[cfg(target_os = "linux")]
    pub fn get_current_session(&self) -> Result<Session, String> {
        let player = PlayerFinder::new()
            .unwrap()
            .find_active()
            .map_err(|_| "Failed to find active MPRIS player".to_string())?;

        Ok(Session { player })
    }

    /// macOS stub
    #[cfg(target_os = "macos")]
    pub fn get_current_session(&self) -> Result<Session, String> {
        Err("Media control not yet implemented for this platform".to_string())
    }
}

impl Session {
    /// Get the current playback status
    #[cfg(target_os = "windows")]
    pub fn get_playback_status(&self) -> Result<PlaybackInfo, String> {
        let playback = self
            .inner
            .GetPlaybackInfo()
            .map_err(|_| "Failed to get playback info".to_string())?;

        let status = playback
            .PlaybackStatus()
            .ok()
            .map(|s| match s {
                GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing => {
                    PlaybackStatus::Playing
                }
                GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused => {
                    PlaybackStatus::Paused
                }
                GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped => {
                    PlaybackStatus::Stopped
                }
                _ => PlaybackStatus::Stopped,
            })
            .unwrap_or(PlaybackStatus::Stopped);

        Ok(PlaybackInfo { status })
    }

    pub fn get_playback_status(&self) -> Result<PlaybackInfo, String> {
        let playback_status = self
            .player
            .get_playback_status()
            .map_err(|_| "Failed to get playback status".to_string())?;

        let status = match playback_status {
            mpris::PlaybackStatus::Playing => PlaybackStatus::Playing,
            mpris::PlaybackStatus::Paused => PlaybackStatus::Paused,
            mpris::PlaybackStatus::Stopped => PlaybackStatus::Stopped,
        };

        Ok(PlaybackInfo { status })
    }

    /// macOS stub
    #[cfg(target_os = "macos")]
    pub fn get_playback_status(&self) -> Result<PlaybackInfo, String> {
        Err("Media control not yet implemented for this platform".to_string())
    }

    /// Get media properties (title, artist, etc.)
    #[cfg(target_os = "windows")]
    pub async fn get_media_properties(&self) -> Result<MediaProperties, String> {
        let media_props = self
            .inner
            .TryGetMediaPropertiesAsync()
            .map_err(|_| "Failed to get media properties".to_string())?
            .await
            .map_err(|_| "Failed to await media properties".to_string())?;

        let title = media_props.Title().ok().unwrap_or_default();

        Ok(MediaProperties { title })
    }

    pub async fn get_media_properties(&self) -> Result<MediaProperties, String> {
        let title = self
            .player
            .get_metadata()
            .map_err(|_| "Failed to get media metadata".to_string())?
            .get("xesam:title")
            .and_then(|v| v.as_str())
            .unwrap_or_default()
            .to_string();

        Ok(MediaProperties { title })
    }

    /// macOS stub
    #[cfg(target_os = "macos")]
    pub async fn get_media_properties(&self) -> Result<MediaProperties, String> {
        Err("Media control not yet implemented for this platform".to_string())
    }

    /// Get timeline properties (current position and duration)
    #[cfg(target_os = "windows")]
    pub fn get_timeline_properties(&self) -> Result<TimelineProperties, String> {
        let timeline = self
            .inner
            .GetTimelineProperties()
            .map_err(|_| "Failed to get timeline properties".to_string())?;

        let position = timeline.Position().ok().map(|ts| ts.Duration).unwrap_or(0);
        let end_time = timeline.EndTime().ok().map(|ts| ts.Duration).unwrap_or(0);

        Ok(TimelineProperties { position, end_time })
    }

    pub fn get_timeline_properties(&self) -> Result<TimelineProperties, String> {
        let position = (self
            .player
            .get_position()
            .map_err(|_| "Failed to get current position".to_string())?
            .as_nanos()
            / 100) as i64;

        let end_time = (self
            .player
            .get_metadata()
            .map_err(|_| "Failed to get track metadata".to_string())?
            .length()
            .unwrap_or_default()
            .as_nanos()
            / 100) as i64;

        Ok(TimelineProperties { position, end_time })
    }

    /// macOS stub
    #[cfg(target_os = "macos")]
    pub fn get_timeline_properties(&self) -> Result<TimelineProperties, String> {
        Err("Media control not yet implemented for this platform".to_string())
    }
}

// Re-export public API
pub fn get_current_session(_manager: &Manager) -> Result<Session, String> {
    _manager.get_current_session()
}

pub fn get_playback_status(session: &Session) -> Result<PlaybackInfo, String> {
    session.get_playback_status()
}

pub async fn get_media_properties(session: &Session) -> Result<MediaProperties, String> {
    session.get_media_properties().await
}

pub fn get_timeline_properties(session: &Session) -> Result<TimelineProperties, String> {
    session.get_timeline_properties()
}
