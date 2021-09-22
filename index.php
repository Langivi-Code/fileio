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

$promise = new Promise(function ($resolve, $reject) {
    setTimeout(fn()=>$resolve("promise finshed"), 3000);

});

setTimeout(fn()=>var_dump("setTimeout finshed"), 2000);

$promise->then(
    fn(...$args) => var_dump($args)
);

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
