/*  Ekos Live Client

    Copyright (C) 2018 Jasem Mutlaq <mutlaqja@ikarustech.com>

    Message Channel

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#pragma once

#include <QMap>

namespace EkosLive
{
enum COMMANDS
{
    GET_CONNECTION,
    GET_STATES,
    GET_CAMERAS,
    GET_MOUNTS,
    GET_SCOPES,
    GET_FILTER_WHEELS,
    NEW_CONNECTION_STATE,
    NEW_MOUNT_STATE,
    NEW_CAPTURE_STATE,
    NEW_GUIDE_STATE,
    NEW_FOCUS_STATE,
    NEW_ALIGN_STATE,
    NEW_POLAR_STATE,
    NEW_PREVIEW_IMAGE,
    NEW_VIDEO_FRAME,
    NEW_ALIGN_FRAME,
    NEW_NOTIFICATION,
    NEW_TEMPERATURE,

    LOGOUT,

    // Profiles
    GET_PROFILES,
    START_PROFILE,
    STOP_PROFILE,

    // Capture
    CAPTURE_PREVIEW,
    CAPTURE_TOGGLE_VIDEO,
    CAPTURE_START,
    CAPTURE_STOP,
    CAPTURE_GET_SEQUENCES,
    CAPTURE_ADD_SEQUENCE,
    CAPTURE_REMOVE_SEQUENCE,

    // Mount
    MOUNT_PARK,
    MOUNT_UNPARK,
    MOUNT_ABORT,
    MOUNT_SYNC_RADE,
    MOUNT_SYNC_TARGET,
    MOUNT_GOTO_RADE,
    MOUNT_GOTO_TARGET,
    MOUNT_SET_MOTION,
    MOUNT_SET_TRACKING,
    MOUNT_SET_SLEW_RATE,

    // Focus
    FOCUS_START,
    FOCUS_STOP,
    FOCUS_RESET,

    // Guide
    GUIDE_START,
    GUIDE_STOP,
    GUIDE_CLEAR,

    // Align
    ALIGN_SOLVE,
    ALIGN_STOP,
    ALIGN_LOAD_AND_SLEW,
    ALIGN_SELECT_SCOPE,
    ALIGN_SELECT_SOLVER_TYPE,
    ALIGN_SELECT_SOLVER_ACTION,
    ALIGN_SET_FILE_EXTENSION,
    ALIGN_SET_CAPTURE_SETTINGS,

    // Polar Assistant Helper
    PAH_START,
    PAH_STOP,
    PAH_REFRESH,
    PAH_SET_MOUNT_DIRECTION,
    PAH_SET_MOUNT_ROTATION,
    PAH_SET_CROSSHAIR,
    PAH_SELECT_STAR_DONE,
    PAH_REFRESHING_DONE,

    // Options
    OPTION_SET_HIGH_BANDWIDTH,
    OPTION_SET_IMAGE_TRANSFER,
    OPTION_SET_NOTIFICATIONS,
    OPTION_SET_CLOUD_STORAGE,

    // Storage Options
    SET_BLOBS,
};

static QMap<COMMANDS, QString> const commands =
{
    {GET_CONNECTION, "get_connection"},
    {GET_PROFILES, "get_profiles"},
    {GET_STATES, "get_states"},
    {GET_CAMERAS, "get_cameras"},
    {GET_MOUNTS, "get_mounts"},
    {GET_SCOPES, "get_scopes"},
    {GET_FILTER_WHEELS, "get_filter_wheels"},
    {NEW_CONNECTION_STATE, "new_connection_state"},
    {NEW_MOUNT_STATE, "new_mount_state"},
    {NEW_CAPTURE_STATE, "new_capture_state"},
    {NEW_GUIDE_STATE, "new_guide_state"},
    {NEW_FOCUS_STATE, "new_focus_state"},
    {NEW_ALIGN_STATE, "new_align_state"},
    {NEW_POLAR_STATE, "new_polar_state"},
    {NEW_PREVIEW_IMAGE, "new_preview_image"},
    {NEW_VIDEO_FRAME, "new_video_frame"},
    {NEW_ALIGN_FRAME, "new_align_frame"},
    {NEW_NOTIFICATION, "new_notification"},
    {NEW_TEMPERATURE, "new_temperature"},

    {LOGOUT, "logout"},


    {START_PROFILE, "profile_start"},
    {STOP_PROFILE, "profile_stop"},

    {CAPTURE_PREVIEW, "capture_preview"},
    {CAPTURE_TOGGLE_VIDEO, "capture_toggle_video"},
    {CAPTURE_START, "capture_start"},
    {CAPTURE_STOP, "capture_stop"},
    {CAPTURE_GET_SEQUENCES, "capture_get_sequences"},
    {CAPTURE_ADD_SEQUENCE, "capture_add_sequence"},
    {CAPTURE_REMOVE_SEQUENCE, "capture_remove_sequence"},

    {MOUNT_PARK, "mount_park"},
    {MOUNT_UNPARK, "mount_unpark"},
    {MOUNT_ABORT, "mount_abort"},
    {MOUNT_SYNC_RADE, "mount_sync_rade"},
    {MOUNT_SYNC_TARGET, "mount_sync_target"},
    {MOUNT_GOTO_RADE, "mount_goto_rade"},
    {MOUNT_GOTO_TARGET, "mount_goto_target"},
    {MOUNT_SET_MOTION, "mount_set_motion"},
    {MOUNT_SET_TRACKING, "mount_set_tracking"},
    {MOUNT_SET_SLEW_RATE, "mount_set_slew_rate"},

    {FOCUS_START, "focus_start"},
    {FOCUS_STOP, "focus_stop"},
    {FOCUS_RESET, "focus_reset"},

    {GUIDE_START, "guide_start"},
    {GUIDE_STOP, "guide_stop"},
    {GUIDE_CLEAR, "guide_clear"},

    {ALIGN_SOLVE, "align_solve"},
    {ALIGN_STOP, "align_stop"},
    {ALIGN_LOAD_AND_SLEW, "align_load_and_slew"},
    {ALIGN_SELECT_SCOPE, "align_select_scope"},
    {ALIGN_SELECT_SOLVER_TYPE, "align_select_solver_type"},
    {ALIGN_SELECT_SOLVER_ACTION, "align_select_solver_action"},
    {ALIGN_SET_FILE_EXTENSION, "align_set_file_extension"},
    {ALIGN_SET_CAPTURE_SETTINGS, "align_set_capture_settings"},

    {PAH_START, "polar_start"},
    {PAH_STOP, "polar_stop"},
    {PAH_REFRESH, "polar_refresh"},
    {PAH_SET_MOUNT_DIRECTION, "polar_set_mount_direction"},
    {PAH_SET_MOUNT_ROTATION, "polar_set_mount_rotation"},
    {PAH_SET_CROSSHAIR, "polar_set_crosshair"},
    {PAH_SELECT_STAR_DONE, "polar_star_select_done"},
    {PAH_REFRESHING_DONE, "polar_refreshing_done"},

    {OPTION_SET_HIGH_BANDWIDTH, "option_set_high_bandwidth"},
    {OPTION_SET_IMAGE_TRANSFER, "option_set_image_transfer"},
    {OPTION_SET_NOTIFICATIONS, "option_set_notifications"},
    {OPTION_SET_CLOUD_STORAGE, "option_set_cloud_storage"},

    {SET_BLOBS, "set_blobs"},
};

}
