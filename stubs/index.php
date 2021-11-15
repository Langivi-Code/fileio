<?php
function test(PromiseStatus $status)
{
    var_dump($status, $status == PromiseStatus::Pending);
}

//
// $serv = new Server(8100, callback:fn()=>1);
// $serv->on_data(function (...$arg){
//    var_dump("on_data8100",$arg);
//               }
//               );
// var_dump($serv);
$file = file_get_contents("./stubs/index.html");
$serv1 = new HttpServer(8000, "tcp://0.0.0.0", []);

$serv1->on_request(function (HttpRequest $req, HttpResponse $res) use ($file) {
$len = strlen($file);
    $str = <<<END
HTTP/1.1 200 OK
content-type: text/html; charset=utf-8
content-length: $len
strict-transport-security: max-age=15552000
x-frame-options: SAMEORIGIN

END;
    $fail = <<<END
HTTP/1.1 403 Forbidden

Nothing to send
END;
    echo "Request details\n";
    echo "Request uri {$req->uri}\n";
if ($req->uri !=="/"){
echo $fail;
    $res->send($fail);
    return;
}

    $str.="\r\n";
    $str.= $file;
//     var_dump($res);
//     $req->method="strange";
//     var_dump($req, $req->query);
    $res->send($str);
//     $res->end("");
//      if ($data == "get_cool"){

//    $serv1->write($str);
//      $serv1->end("");
//      }


    //                $serv1->write(" from php hello");
});
// $serv1->write("from php hello");
// $serv1->write($str);
// var_dump($serv1);

// idle(fn()=>print_r("hello"));
// idle(fn()=>print_r("hello2"));
// idle(fn()=>print_r("hello3"));
// $serv1 = new Server(8200, callback: fn() => 1);
// $str = "HTTP/1.1 200 OK\n\r";
// $serv1->on_data(function ($data) use (&$serv1) {
//     if ($data == "get_cool")
//          $serv1->write("cool");
//     var_dump("on_data8200: " . $data);
//
// //                $serv1->write(" from php hello");
// });
// $serv1->write("from php hello");
// $serv1->write($str);
// var_dump($serv1);
// $serv2 = new Server(8005,"0.0.0.0", fn()=>var_dump('on_connect'));
// $serv2->on_data("work");
// $str = stream_socket_server("tcp://0.0.0.0:8004");
// $i =0;
// while (true){
//    $con = stream_socket_accept($str);
//    $i++;
//    echo fread($con, 1024);
//    stream_socket_sendto($con,"HTTP/1.1 200 OK\n", STREAM_OOB);
// //    echo  $i."\n";
//    fclose($con);
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
//$socket = socket_create(AF_INET, SOCK_STREAM, 0);
//var_dump(stream_get_transports());
/* Open a server socket to port 1234 on localhost */

//$server = stream_socket_server('tcp://127.0.0.1:1234');
//
///* Accept a connection */
//$socket = stream_socket_accept($server);
//stream_set_blocking($server, false);
///* Grab a packet (1500 is a typical MTU size) of OOB data */
//echo "Received Out-Of-Band: '" . stream_socket_recvfrom($socket, 1500, STREAM_OOB) . "'\n";
//
///* Take a peek at the normal in-band data, but don't consume it. */
//echo "Data1: '" . stream_socket_recvfrom($socket, 1500, STREAM_PEEK) . "'\n";
//
///* Get the exact same packet again, but remove it from the buffer this time. */
//echo "Data2: '" . stream_socket_recvfrom($socket, 1500) . "'\n";
//$serv= [$server];
//echo stream_select($serv);
///* Close it up */
//fclose($socket);
//fclose($server);

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

