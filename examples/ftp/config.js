const fs = require('fs');
const path = require('path');

let step = [false, false, false];
let current = 0;
let funcs = [getPath, getName, getPassword];
process.stdin.on('readable', () => {
  let chunk = process.stdin.read();
  if (chunk !== null) funcs[current]('' + chunk);
});
process.stdout.write(`请输入共享文件夹的绝对路径\n`);


function getPath(str) {
  try {
    if (!path.isAbsolute(str)) {
      process.stdout.write(`输入不合法\n`);
    } else {
      step[0] = str;
      ++current;
      process.stdout.write(`请输入用户名，仅包含大小写字母和数字\n`);
    }
  } catch(err) {
    process.stdout.write(`输入不合法\n`);
  }
}

function getName(str) {
  if (!/[a-zA-Z0-9]/.test(str)) {
    process.stdout.write(`输入不合法\n`);
  } else {
    step[1] = str;
    ++current;
    process.stdout.write(`请输入密码，仅包含大小写字母和数字\n`);
  }
}

function getPassword(str) {
  if (!/[a-zA-Z0-9]/.test(str)) {
    process.stdout.write(`输入不合法\n`);
  } else {
    step[2] = str;
    writeToFile();
  }
}

function writeToFile() {
  let temp = '';
  for (let i = 0; i < 3; i++) temp += step[i];
  fs.writeFile('/tmp/fileSync/config', temp, err => {
    if (err) throw err;
    process.stdout.write(`finish\nenjoy~\n`);
    process.exit(0);
  });
}
