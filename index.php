<?php
//echo var_dump(use_promise());
//require_once "Promise.php";

ini_set('memory_limit', '1024M');


//setTimeout(function ()  {
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
setTimeout(fn()=>var_dump(124), 10000);
file_get_contents_async("config.h", fn($arg)=>file_put_contents("dtad33", $arg));
idle(fn()=>var_dump(123));
// file_get_contents_async("configure", fn($arg)=>file_put_contents("dtad2", $arg));

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


// setTimeout(function(){
//   sleep(1);
// echo "bye";
// foreach(["mama", "papa","syn"] as $key=>$value){
// var_dump($key, $value);
// }
// }, 1000);
echo "hello6786486787695686784875979\n";

sleep(1);
