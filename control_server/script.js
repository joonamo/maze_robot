var last_status = {raw_status: "connecting..."};
var status_div;
var bar_l;
var bar_f;
var bar_r;

function display_dist(bar, dist) {
    bar.css("height", Math.min(dist / 500 * 100, 100) + "%");

}

function display_last_status() {
    status_div.html(last_status.raw_status);
    display_dist(bar_l, last_status.dist_l);
    display_dist(bar_f, last_status.dist_f);
    display_dist(bar_r, last_status.dist_r);
}

function get_status() {
    $.ajax({
        method: "GET",
        url: "http://teensypi.local:8000/",
        dataType: "json",
        success: function(data){
            last_status = data.status;
            display_last_status();
            get_status();
        },
        error: function(jqXHR, textStatus, errorThrown)
        {
            last_status = {raw_status: "Error: " + textStatus + ", " + errorThrown};
            display_last_status();
            setTimeout( get_status, 5000 );
        } 

    });
}

$(document).ready(function() 
{
    status_div = $("#status");
    bar_l = $("#bar_l");
    bar_f = $("#bar_f");
    bar_r = $("#bar_r");
    display_last_status();
    get_status();
});