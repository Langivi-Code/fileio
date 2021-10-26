// setTimeout(function () {
//     console.log( "timeout1")
// }, 0);
//
//
// setImmediate(function () {
//     console.log("idle111");
// });
//
// setImmediate(function () {
//     console.log(setTimeout(function () {
//         console.log( "timeout3")
//     }, 0) + "timer id\n");
//     console.log("idle2***********");
// });
//
// setTimeout(function () {
//     console.log( "timeout2")
// }, 0);

const net = require('net');
const sock = new net.Socket({readable:true, writable:true});

sock.connect(8004, '0.0.0.0');
sock.on('connect', (...data)=>{console.log('connect', data)
sock.write("hell7777t6fi67fididi76du65du6iu6fir6yidkil77rfdur6iliduxiuo\n\0");
 // sock.end();
});
sock.on('data', (...data)=>console.log('data', data.toString()));
// let i = 0;
// const serv = net.createServer((socket) => {
//     // console.log('req', i);
//     // i++;
//     socket.end('HTTP/1.1 200 OK\n');
//
// })
// serv.listen(8004);
// 11410470
//  1201680
//   731087
