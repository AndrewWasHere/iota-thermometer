/*
    Settings in mm
*/
hanger_screw_head_diameter = 10;
hanger_screw_shaft_diameter = 5;
nut_diameter = 6;  // vertex-to-vertex diameter
nut_height = 2;
standoff_id = 3;
standoff_od = 5;
standoff_screw_length = 8.5;  // below the head length

function base_measurements() = [60, 60];
function display_measurements() = [45.5 / 2, 18 / 2];
function thermometer_measurements() = [20.5 / 2, 12.5 / 2];
function display_position() = [0, 10, 0];
function thermometer_position() = [0, -17, 0];

$fn = 20;    

/*
    hanger
    
    Parameters:
    head_diameter: diameter of screw / bolt head.
    shaft_diameter: diameter of screw / bolt shaft.
*/
module hanger(head_diameter, shaft_diameter) {
    circle(d=shaft_diameter);
    translate([0, -shaft_diameter / 2, 0]) {
        square(shaft_diameter, center=true);
    }
    translate([0, -shaft_diameter * 1.5, 0]) {
        circle(d=head_diameter);
    }
}

/*
    donut

    Parameters:
    od: outer diameter
    id: inner diameter
*/
module donut(od, id) {
    difference() {
        circle(d=od);
        circle(d=id);
    }
}

/*
    displaced_circle

    Parameters:
    offset: [x, y, z] to translate
    d: diameter of circle
*/
module displaced_circle(offset, d, fn=0) {
    translate(offset) circle(d=d, $fn=fn);
}

/*
    displaced_donut

    Parameters:
    offset: [x, y, z] to translate
    od: diameter of outer circle
    id: diameter of inner circle
*/
module displaced_donut(offset, od, id) {
    translate(offset) donut(od, id);
}

/*
    display_hole_template

    Parameters:
    d: diameter of circle
    fn: number of render lines for circle
*/
module display_hole_template(d, fn=0) {
    p = display_measurements();
    x = p[0];
    y = p[1];
    displaced_circle([-x, y, 0], d, fn);
    displaced_circle([-x, -y, 0], d, fn);
}

/*
    display_standoff_template

    Parameters:
    od: outer diameter
    id: inner diameter
*/
module display_standoff_template(od, id) {
    p = display_measurements();
    x = p[0];
    y = p[1];
    displaced_donut([-x, y, 0], od, id);
    displaced_donut([-x, -y, 0], od, id);
    displaced_circle([x, y, 0], id, $fn);
    displaced_circle([x, -y, 0], id, $fn);
}    

/*
    thermometer_hole_template

    Parameters:
    d: diameter of hole
*/
module thermometer_hole_template(d, fn=0) {
    p = thermometer_measurements();
    x = p[0];
    y = p[1];
    for (offset = [[-x, y, 0], [-x, -y, 0], [x, y, 0], [x, -y, 0]]) {
        displaced_circle(offset, d, fn);
    }
}

/*
    thermometer_standoff_template

    Parameters:
    od: outer diameter
    id: inner diameter
*/
module thermometer_standoff_template(od, id) {
    p = thermometer_measurements();
    x = p[0];
    y = p[1];
    for (offset = [[-x, y, 0], [-x, -y, 0], [x, y, 0], [x, -y, 0]]) {
        displaced_donut(offset, od, id);
    }
}

/*
    standoffs

    Parameters:
    od: outer diameter
    id: inner diameter
*/
module standoffs(od, id) {
    translate(display_position()) display_standoff_template(standoff_od, standoff_id);
    translate(thermometer_position()) thermometer_standoff_template(standoff_od, standoff_id);
}

/*
    base

    2D base with hanger hole and PCB mounting holes

    Parameters:
    d: hole diameter
    fn: number of render lines for circle
*/
module base(d, fn) {
    p = base_measurements();
    difference() {
        minkowski() {
            square(p, center=true);
            circle(2);
        }
        translate([0, (p[1] / 2) - 5, 0]) hanger(hanger_screw_head_diameter, hanger_screw_shaft_diameter);
        translate(display_position()) display_hole_template(d, fn=fn);
        translate(thermometer_position()) thermometer_hole_template(d, fn=fn);
    }
}

/*
    base_nuts

    2D base with holes for nuts
*/
module base_nuts() {
    base(nut_diameter, 6);
}

/*
    base_passthru

    2D base with holes for passthroughs
*/
module base_passthru() {
    base(standoff_id, $fn);
}

/*
    plate

    3D render of entire plate
*/
module plate() {
    z1 = nut_height;
    z2 = 1;
    z3 = standoff_screw_length - (z1 + z2);
    
    linear_extrude(height=z1, center=false) base_nuts();
    translate([0, 0, z1]) linear_extrude(height=z2, center=false) base_passthru();
    translate([0, 0, z1 + z2]) linear_extrude(height=z3, center=false) standoffs();
}

plate();