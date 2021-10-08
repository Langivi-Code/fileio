<?php
//echo var_dump(use_promise());
//require_once "Promise.php";


// set_timeout(function ()  {
//        sleep(1);
//        echo "bye";
//        foreach ([1, 2, 3, 4, 5] as $key => $value) {
//            var_dump($key, $value);
//        }
//    }, 0);
//

// $promise = new Promise(function ($resolve, $reject) {
//     setTimeout(fn()=>$resolve("promise finshed"), 3000);
//
// });

// echo $timer = setInterval(fn()=>var_dump("setTimeout finished"), 200);
// echo "<br>$timer</br>";

file_get_contents_async("compile",
    fn($arg2) => file_put_contents("dtad34", $arg2)&&var_dump("first callback"));
//file_put_contents_async("compile2", "data");
//sleep(3);
file_get_contents_async("Makefile", fn($arg) => file_put_contents("dtad35", $arg)&&var_dump("second callback"));

// file_get_contents_async("fileio.lo", function($arg){
// var_dump("******************************", $arg);
// }, maxlen:160);
//
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
//setTimeout(function () {
//    echo "timeout2";
//}, 0);
file_get_contents_async(
    "compile",
    fn($arg) => var_dump("dtad336")&var_dump("third callback")
);

// file_get_contents_async(
//     "compile",
//     fn($arg)=>print_r("dtad336")
//  );
//
// //  file_get_contents_async(
// //     "compile",
// //     fn($arg)=>print_r("dtad336")
// //  );
//  file_get_contents_async(
//      "README.md",
//      fn($arg)=>var_dump($arg,123, "second cb")//file_put_contents("dtad337", $arg)
//   );
echo "sync exec ended.\n\n";

