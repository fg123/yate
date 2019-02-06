import math
import io
import string
import list

struct Color => (r, g, b);
Color.init => (hex_str) {
    let hexToNum => (x) {
        let res = 0;
        for c in x {
            res *= 16;
            if (c.val >= '0'.val and c.val <= '9'.val)
                res += c.val - '0'.val;
            else if (c.val >= 'A'.val and c.val <= 'Z'.val)
                res += 10 + (c.val - 'A'.val);
            else if (c.val >= 'a'.val and c.val <= 'z'.val)
                res += 10 + (c.val - 'a'.val);
        }
        ret res;
    };
    this.r = hexToNum(hex_str[0->2]);
    this.g = hexToNum(hex_str[2->4]);
    this.b = hexToNum(hex_str[4->6])
    ret this;
}

let @<Color> => (c) c.r + ", " + c.g + ", " + c.b

let <Color> - <Color> => (l, r) sqrt(pow(l.r - r.r, 2) + pow(l.g - r.g, 2) + pow(l.b - r.b, 2))

let min => (a, b) ret if a < b a else b;

let xtermColors = map(Color, io.readFile("xterm-colors.txt") / "\n");
for true {
    @"Which color do you want to search for: ";
    let result = io.readRaw();
    let desired_color = Color(result);
    let closest_color = 0;
    for i in 0->xtermColors.size
        if (xtermColors[i] - desired_color < xtermColors[closest_color] - desired_color)
            closest_color = i;
    @"The closest color to ";
    @desired_color;
    @" is at index " + closest_color + " and is: ";
    xtermColors[closest_color];
}



