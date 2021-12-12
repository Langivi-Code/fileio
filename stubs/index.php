<?php
function test(PromiseStatus $status)
{
    var_dump($status, $status == PromiseStatus::Pending);
}

//$file = file_get_contents("./stubs/index.html");
//$serv1 = new HttpServer(8001, "tcp://0.0.0.0", []);

//$serv1->on_request(function (HttpRequest $req, HttpResponse $res) {
////  var_dump($_POST);
////     var_dump($req);
////    $len = strlen($file);
////    echo "Request details\n";
////    echo "Request uri {$req->uri}\n";
//    //$res->statusCode=2;
////    var_dump($res);
////    if ($req->uri !== "/") {
////        echo "Nothing to send";
////        $res->setStatusCode(403);
////         var_dump($res);
////
////
//////        $res->setHeader("content-length", "0");
////        $res->send("\r\nNothing to send");
////        return;
////    } else {
//        $res->setHeader("content-length", "10");
//        $res->setHeader("Content-Type", "text/plain; charset=utf-8");
//        $res->send("hello\n");
////    }
//});
// $serv1->write("from php hello");
// $serv1->write($str);
// var_dump($serv1);

// idle(fn()=>var_dump("hello"));
// idle(fn()=>var_dump("hello2"));
// idle(fn()=>var_dump("hello3"));
//
//
//
//$ret = new Promise(function ($resolve, $reject)  use (&$ret){
//   var_dump($reject("I am rejected prom promise\n") );
////     $reject(123);
//    var_dump($ret);
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
//     set_timeout(fn()=>$resolve("promise finshed"), 3000);
//
// });

// echo $timer = setInterval(fn()=>var_dump("setTimeout finished"), 200);
// echo "<br>$timer</br>";
// //

// $timerId = new Promise(function ($resolve, $reject){
// //    $resolve(123);
//     file_get_contents_async("serv.php", $resolve);
// });
//
// $timerId->then(function ($data) {
//     var_dump($data);
// //    var_dump($timerId);
// });
Promise::resolve(1)->then(function($i){
                            var_dump($i);
                            });
Promise::reject("frsrgrgrrv")->catch(function($i){
var_dump($i);
});
//var_dump($timerId);


 //file_put_contents_async("compile2", "data");
 ////sleep(3);
// file_get_contents_async("Makefile", fn($arg) => file_put_contents("dtad35", $arg)&&var_dump("second callback"));
// //
// file_get_contents_async("fileio.lo", function($arg){
// var_dump("******************************", $arg)&&var_dump("four callback");
// }, maxlen:100);

// file_get_contents_async("fileio.la", function($arg){
// var_dump("******************************", $arg);
// }, maxlen:160);
// idle(fn()=>var_dump(123));
// file_put_contents_async("configure1", "fn(arg)=>file_put_contents(dtad2, arg)", fn()=>var_dump("dtad337"));


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
/* Open a server socket to port 1234 on localhost */


//file_get_contents_async(
//    "compile",
//    fn($arg) => var_dump("dtad336")&var_dump("third callback")
//);

echo "sync exec ended.\n\n";

