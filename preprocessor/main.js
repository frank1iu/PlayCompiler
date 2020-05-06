const readline = require("readline");
const reader = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: false
});

function init(lines) {
    global.$ = {};
    lines = lines.reverse();
    function dequeue() {
        return lines.pop().trim();
    }
    function replace_defined_values(line) {
        for (const key of dict.keys()) {
            const escaped = key.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&');
            const regex = new RegExp(escaped, "g");
            line = line.replace(regex, eval(dict.get(key)));
        }
        return line;
    }
    let curr;
    let ret = [];
    let dict = new Map();
    while (lines.length !== 0) {
        curr = dequeue();
        if (curr.startsWith(";#")) {
            const op = curr.split(" ")[0].substring(2);
            switch (op) {
                case "define":
                    const define_name = curr.split(" ")[1];
                    const define_code = curr.split(" ").slice(2).join(" ");
                    dict.set(define_name, define_code);
                    break;
                case "eval":
                    const eval_code = curr.split(" ").slice(1).join(" ");
                    eval(eval_code);
                    break;
                case "if":
                    const if_code = curr.split(" ").slice(1).join(" ");
                    const if_cond = eval(replace_defined_values(if_code));
                    curr = dequeue();
                    while (!curr.startsWith(";#endif")) {
                        if (if_cond) {
                            ret.push(replace_defined_values(curr));
                        }
                        curr = dequeue();
                    }
                    break;
            }
        } else {
            if (curr.includes(";")) {
                const index = curr.indexOf(";");
                curr = curr.substring(0, index);
            }
            ret.push(replace_defined_values(curr));
        }
    }
    console.log(ret.filter(line => line !== "").join(" \n"));
    return ret.filter(line => line !== "").join(" \n");
}

let lines = [];

reader.on("line", function(line) {
    lines.push(line);
});

reader.on("close", function() {
    init(lines);
});