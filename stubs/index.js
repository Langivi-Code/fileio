setTimeout(function () {
    console.log( "timeout1")
}, 0);


setImmediate(function () {
    console.log("idle111");
});

setImmediate(function () {
    console.log(setTimeout(function () {
        console.log( "timeout3")
    }, 0) + "timer id\n");
    console.log("idle2***********");
});

setTimeout(function () {
    console.log( "timeout2")
}, 0);
