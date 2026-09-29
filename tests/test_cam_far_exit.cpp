// test_cam_far_exit.cpp - the C up view left by C-down far from the origin, held to the console.
//
// At z = -204009 an f32 steps in 1/64ths. The C up view holds the eye 20 units from the centre,
// so the rounding of the eye's z there is worth several s16 of yaw, and the exit csangle lands 10
// above the facing where at the origin it lands on it. Nothing in the camera is wrong: the game
// rounds the same way.
//
// The tape: Link standing at (-144.753647, 668.66925, -204009.375) on flat ground at his y, facing
// 15672. 40 frames in the C up view, the C-stick down 4 frames, 60 frames still. The seat does not
// matter: every camera yaw it was tried from exits on the same value.
//
// Console tier: read live on GZLJ01 (`dCam_getControledAngleY`) after the same input at that
// spot, sea room 11, facing 15672.

#include <string>

#include "framework/require_bits.h"
#include "framework/runner.h"

#include "boundary/game_boundary.h"
#include "d/actor/d_a_player_main.h"
#include "engine/session.h"
#include "m_Do/m_Do_controller_pad.h"

namespace {

int exit_csangle(float x, float y, float z, int facing, int seat) {
    tww_engine::Init init;
    init.pos.set(x, y, z);
    init.shape_angle_y = static_cast<s16>(facing);
    init.travel_angle_y = static_cast<s16>(facing);
    init.normal_speed = 0.0f;
    init.speed_f = 0.0f;
    init.proc = daPy_lk_c::daPyProc_WAIT_e;
    init.pad.stick_angle_raw = static_cast<s16>(facing);
    init.pad.target_angle = static_cast<s16>(facing);
    tww_engine::RunOptions opts;
    opts.ground = tww_engine::RunOptions::Ground::Supplied;
    opts.floor_y = y;
    opts.camera = true;
    opts.camera_yaw = static_cast<s16>(seat);
    tww_engine::Session session(init, NULL, opts);
    session.bind();
    tww_engine::Pad pad;
    pad.stick_angle_raw = static_cast<s16>(facing);
    pad.target_angle = static_cast<s16>(facing);
    auto one = [&](bool view, float cstick_y) {
        g_mDoCPd_cpadInfo[PAD_1].mMainStickPosX = 0.0f;
        g_mDoCPd_cpadInfo[PAD_1].mMainStickPosY = 0.0f;
        g_mDoCPd_cpadInfo[PAD_1].mMainStickValue = 0.0f;
        g_mDoCPd_cpadInfo[PAD_1].mCStickPosX = 0.0f;
        g_mDoCPd_cpadInfo[PAD_1].mCStickPosY = cstick_y;
        g_mDoCPd_cpadInfo[PAD_1].mCStickValue = cstick_y < 0.0f ? -cstick_y : cstick_y;
        g_mDoCPd_cpadInfo[PAD_1].mTriggerLeft = 0.0f;
        cam_stub_lockon = false;
        pad.attention_lock = FALSE;
        if (view) {
            session.lk.stub_player_status0 |= daPyStts0_SUBJECT_e;
        } else {
            session.lk.stub_player_status0 &= ~daPyStts0_SUBJECT_e;
        }
        session.step(pad);
    };
    for (int i = 0; i < 11; ++i) one(false, 0.0f);
    for (int i = 0; i < 40; ++i) one(i >= 3, 0.0f);
    for (int i = 0; i < 10; ++i) one(i < 4, -1.0f);
    for (int i = 0; i < 60; ++i) one(false, 0.0f);
    g_mDoCPd_cpadInfo[PAD_1].mCStickPosY = 0.0f;
    g_mDoCPd_cpadInfo[PAD_1].mCStickValue = 0.0f;
    return session.cameraFacts().yaw & 0xFFFF;
}

}  // namespace

TWWE_TEST(the_c_up_exit_far_from_the_origin_is_the_consoles,
          "a camera table built at the origin is off by 10 at z -204009, and a camera computed in "
          "wider floats than the game's would agree with the table instead of the console") {
    const int seats[] = {15672, 0, 30000};
    for (int seat : seats) {
        REQUIRE_INT(c, exit_csangle(-144.753647f, 668.66925f, -204009.375f, 15672, seat), 15682,
                    "the exit at his spot, seated at " + std::to_string(seat));
    }
    REQUIRE_INT(c, exit_csangle(0.0f, 0.0f, 0.0f, 15672, 15672), 15672,
                "the exit at the origin (the engine's own, not read on console)");
}
