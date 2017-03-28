var last_status = {raw_status: "connecting..."};
var status_div;
var bar_l;
var bar_f;
var bar_r;
var manual_checkbox;

var direction_controller;
var speed_controller;
var direction_knob;
var speed_knob;

var control_channel_free = true;

var update_active;
var update_active_checkbox;

function clamp(v, mi, ma) {
    return Math.min(ma, Math.max(mi, v));
}

function display_dist(bar, dist) {
    bar.css("height", Math.min(dist / 500 * 100, 100) + "%");

}

function display_last_status() {
    status_div.html(last_status.raw_status);
    display_dist(bar_l, last_status.dist_l);
    display_dist(bar_f, last_status.dist_f);
    display_dist(bar_r, last_status.dist_r);
    manual_checkbox.prop('checked', last_status.manual > 0);
}

function receive_data(data){
    last_status = Object.assign(last_status, data.status);
    display_last_status();
}

function get_status() {
    if (update_active)
    {
        $.ajax({
            method: "GET",
            url: "status",
            dataType: "json",
            success: function(data){
                receive_data(data);
                setTimeout(get_status, 1);
            },
            error: function(jqXHR, textStatus, errorThrown)
            {
                last_status = {raw_status: "Error: " + textStatus + ", " + errorThrown};
                display_last_status();
                setTimeout( get_status, 5000 );
            } 
        });
    } else {
        setTimeout(get_status, 1000);
    }
}

function toggle_manual() {
    $.ajax({
        method: "GET",
        url: "toggle_manual",
        dataType: "json"
    });
}

function check_update_active() {
    update_active = update_active_checkbox.prop("checked");
}

function dir_knob_mouse(event){
    event.preventDefault();
    if (event.buttons > 0) {
        var p = event.clientX / direction_controller[0].offsetWidth;
        control_dir(p);
    }
}

function dir_knob_touch(event){
    event.preventDefault();
    var p = event.touches[0].clientX / direction_controller[0].offsetWidth;
    control_dir(p);
}

function control_dir(p) {
    p = clamp(p, 0.0, 1.0);
    direction_knob.css("left", clamp(p * 100 - 2, 0, 96) + "%");
    if (control_channel_free)
    {
        control_channel_free = false;
        $.ajax({
            method: "GET",
            url: "status",
            dataType: "json",
            data: {dir: p * 360 - 180},
            success: function(data) {
                receive_data(data);
                control_channel_free = true;
            },
            error: function() {
                control_channel_free = true;
            }
        });
    }
}

function speed_knob_mouse(event){
    event.preventDefault();
    if (event.buttons > 0) {
        var p = (event.clientY - speed_controller[0].offsetTop) / direction_controller[0].offsetHeight;
        control_speed(p);
    }
}

function speed_knob_touch(event){
    event.preventDefault();
    var p = (event.touches[0].clientY - speed_controller[0].offsetTop) / direction_controller[0].offsetHeight;
    control_speed(p);
}

function control_speed(p) {
    p = clamp(p, 0.0, 1.0);
    speed_knob.css("top", clamp(p * 100 - 2, 0, 96) + "%");
    if (control_channel_free)
    {
        control_channel_free = false;
        $.ajax({
            method: "GET",
            url: "status",
            dataType: "json",
            data: {speed: 255 - p * 512},
            success: function(data) {
                receive_data(data);
                control_channel_free = true;
            },
            error: function() {
                control_channel_free = true;
            }
        });
    }
}

$(document).ready(function() 
{
    status_div = $("#status");
    bar_l = $("#bar_l");
    bar_f = $("#bar_f");
    bar_r = $("#bar_r");

    manual_checkbox = $("#manual_checkbox");
    update_active_checkbox = $("#update_active_checkbox");
    update_active_checkbox.change(check_update_active);
    
    direction_controller = $("#direction_controller");
    speed_controller = $("#speed_controller");
    direction_knob = $("#direction_knob");
    speed_knob = $("#speed_knob");

    $("#direction_controller").mousemove(dir_knob_mouse);
    $("#direction_controller").mousedown(dir_knob_mouse);
    $("#direction_controller").on("touchstart", dir_knob_touch);
    $("#direction_controller").on("touchmove", dir_knob_touch);

    $("#speed_controller").mousemove(speed_knob_mouse);
    $("#speed_controller").mousedown(speed_knob_mouse);
    $("#speed_controller").on("touchstart", speed_knob_touch);
    $("#speed_controller").on("touchmove", speed_knob_touch);

    check_update_active();
    display_last_status();
    get_status();
});