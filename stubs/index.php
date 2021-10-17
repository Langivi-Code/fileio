<?php
function test(PromiseStatus $status)
{
    var_dump($status, $status == PromiseStatus::Pending);
}

// class pp{
//     private ?string $ll=null;
// }

//$ret = new Promise(function ($resolve, $reject) {
//   var_dump($reject("I am rejected prom promise\n") );
////     $reject(123);
//});
// var_dump($ret);
// set_timeout(function ()  {
//        sleep(1);
//        echo "bye";
//        foreach ([1, 2, 3, 4, 5] as $key => $value) {
//            var_dump($key, $value);
//        }
//    }, 0);
//
// test(PromiseStatus::Pending);
// $promise = new Promise(function ($resolve, $reject) {
//     setTimeout(fn()=>$resolve("promise finshed"), 3000);
//
// });

// echo $timer = setInterval(fn()=>var_dump("setTimeout finished"), 200);
// echo "<br>$timer</br>";
// //
// file_get_contents_async("compile",
//     fn($arg2) => file_put_contents("dtad34", $arg2)&&var_dump("first callback"));
// //file_put_contents_async("compile2", "data");
// ////sleep(3);
// file_get_contents_async("Makefile", fn($arg) => file_put_contents("dtad35", $arg)&&var_dump("second callback"));
// //
// file_get_contents_async("fileio.lo", function($arg){
// var_dump("******************************", $arg)&&var_dump("four callback");
// }, maxlen:100);

// file_get_contents_async("fileio.la", function($arg){
// var_dump("******************************", $arg);
// }, maxlen:160);
// idle(fn()=>var_dump(123));
 file_put_contents_async("configure1", "fn(arg)=>file_put_contents(dtad2, arg)", fn()=>var_dump("dtad337"));

// setTimeout(fn()=>exit(), 5000);

// $promise->then(
//     fn(...$args) => var_dump($args)
// );

//$promise = new Promise(function ($resolve, $reject) {
//    setTimeout(function () use ($resolve) {
//        sleep(1);
//        echo "bye";
//        foreach ([1, 2, 3, 4, 5] as $key => $value) {
//            var_dump($key, $value);
//        }
//        $resolve("promise finshed");
//    }, 1);
//});
//$promise = Promise::resolve("error");
//$promise->catch(
//    fn(...$args) => var_dump($args)
//);

//$promise->finally(
//    fn() => var_dump("args")
//);
//$promise->finally(
//    fn() => var_dump("args")
//);
//
//
//set_timeout(function () {
//    echo "timeout1";
//}, 100);

//
//idle(function () {
//    echo "idle111";
//});
//
//idle(function () {
//    echo setTimeout(function () {
//            echo "timeout3";
//        }, 1) . "timer id\n";
//    echo "idle2***********";
//});
//
//$fb = new Fiber(function (){
//    echo Fiber::suspend(123);
//    echo  " after resume\n";
//return "returned";
//
//});
//
//echo "in Fiber".$fb->start();
//set_timeout(function () use ($fb){
//    echo "timeout2 \n";
//    echo $fb->resume(1234);
//    echo "\ntimeout3 \n";
//    echo $fb->getReturn();
//}, 0);
//$socket = socket_create(AF_INET, SOCK_STREAM, 0);
//var_dump(stream_get_transports());
/* Open a server socket to port 1234 on localhost */
$server = stream_socket_server('tcp://127.0.0.1:1234');

/* Accept a connection */
$socket = stream_socket_accept($server);
stream_set_blocking($server, false);
/* Grab a packet (1500 is a typical MTU size) of OOB data */
echo "Received Out-Of-Band: '" . stream_socket_recvfrom($socket, 1500, STREAM_OOB) . "'\n";

/* Take a peek at the normal in-band data, but don't consume it. */
echo "Data1: '" . stream_socket_recvfrom($socket, 1500, STREAM_PEEK) . "'\n";

/* Get the exact same packet again, but remove it from the buffer this time. */
echo "Data2: '" . stream_socket_recvfrom($socket, 1500) . "'\n";
$serv= [$server];
echo stream_select($serv);
/* Close it up */
fclose($socket);
fclose($server);

//file_get_contents_async(
//    "compile",
//    fn($arg) => var_dump("dtad336")&var_dump("third callback")
//);

// file_get_contents_async(
//     "compile",
//     fn($arg)=>print_r("dtad336")
//  );
// //
//    file_get_contents_async(
//       "compile",
//       fn($arg)=>print_r("dtad336")
//    );
//   file_get_contents_async(
//       "README.md",
//       fn($arg)=>var_dump($arg,123, "second cb")//file_put_contents("dtad337", $arg)
//    );
echo "sync exec ended.\n\n";

