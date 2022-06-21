<?php
class ExtendedPromise extends \Promise
{
    public static function all (array $promises):\Promise
    {
        $bb = new \Promise(function($resolveAll, $rej) use(&$promises, &$bb){
            $promiseArray = array();
            $lengthPromises = count($promises);
            if($lengthPromises == 0){
                $resolveAll($promiseArray);
            }
            foreach($promises as $key => $promise){
                $promise->then(function($result) use(&$bb, &$lengthPromises,&$promiseArray, $resolveAll, $key){
                    var_dump('promise inside', $result, $key);
                    file_put_contents($result,'');
                    $promiseArray[$key] = $result;
                    $lengthPromises -= 1;
                    if ($lengthPromises == 0){
                        echo "promise all resolved\n";
                        var_dump($promiseArray);
                        $resolveAll($promiseArray);
                        echo "hello1\n";
                    }

                });
//                var_dump($promise);
            }

        });
        return $bb;
    }
}

//$pro = new \Promise(fn($res, $rej) => set_timeout(fn() => $res(-1), 1000));
////var_dump($pro);
//
//$pro->then(function ($a) {
//    var_dump("returned is" . $a);
////    new Promise(function ($res, $rej){  $res(40);});
////    $promise = (new \Promise(fn($res, $rej) => set_timeout(fn() => $res(1), 100)));
////    var_dump($promise);
////    $promise->then('var_dump');
//return new Promise(function ($res, $rej){ $res(30);});
////    set_timeout(function (){
////    return Promise::resolve(2);
////    var_dump($prms);
////    },0);
//})->then(function ($result){
//    echo"-------------------------------------------------------";
//    var_dump($result);
//});
//$pro2 =  ;
////var_dump($pro);
//$pro2->then(fn($do)=>2)->then(fn($do)=>3);
// $promises = [
//    new \Promise(fn($res, $rej) => set_timeout(fn() => $res(1), 100)),
//    new \Promise(fn($res, $rej) => set_timeout(fn() => $res(2), 2000)),
//    new \Promise(fn($res,$rej)=> set_timeout(fn()=>$res(3),3000)) ,
//];
////        var_dump($promises);
//$a = ExtendedPromise::all($promises)->then(function (array $params) {
//    echo "ExtendedPromise resolved\n\n\n";
//    var_dump($params);
////    return $params;
//});

//$a->then(function ($res) use ($a) {
//    echo "qwreqr";
//   var_dump($a, $res);
//});
//set_timeout(fn()=>100, 10000);
function test(PromiseStatus $status)
{
    var_dump($status, $status == PromiseStatus::Pending);
}

//$db = mysqli_connect("raspis00.mysql.tools", "raspis00_bd", "", "raspis00_bd");
//mysqli_query($db, "select 1", MYSQLI_ASYNC);
//function mysql_query(mysqli $coonection, string $query):Promise
//{
//    mysqli_query($coonection, $query, MYSQLI_ASYNC);
//    return new Promise(fn($res, $rej) => mysql_wait($coonection, fn($arg) => $res(mysqli_reap_async_query($arg)))); //TODO rework to promise
//}
//mysql_query($db, "select 1");
$pg = pg_connect("host=0.0.0.0 user=root password=password");
pg_wait($pg,
    fn($connection) => "select 1",
    function ($connection) use ($pg) {
        var_dump(pg_fetch_all($connection));
        pg_wait($pg,
            fn($connection) => "select 2",
            function ($connection) use ($pg) {
                var_dump(pg_fetch_all($connection));
                pg_wait($pg,
                    fn($connection) => "select 3",
                    fn($res) => var_dump(pg_fetch_all($res)));
            } );
} );
pg_wait($pg,
    fn($connection) => "select 3", //WRITE SQL
    fn($res) => var_dump(pg_fetch_all($res))//READ RESULT
    );
});
//pg_wait($pg,
//    fn($connection) => "select 3",
//    fn($connection) => var_dump(pg_fetch_all($connection)));
//pg_wait($pg,
//    fn($connection) => "select 4",
//    fn($connection) => var_dump(pg_fetch_all($connection)));
//function query(\PgSql\Connection $coonection, string $query):Promise
//{
//    pg_send_query($coonection, $query);
//    return new Promise(fn($res, $rej) => pg_wait($coonection, fn($connection) => $res($connection))); //TODO rework to promise
//}
//
//$promsie = query($pg, "select 1 as int");
//var_dump($promsie);
//var_dump($promsie->then(function (\PgSql\Result $arg) {
//    var_dump($arg);
//    var_dump(pg_fetch_all($arg));
//}));

//pg_send_query($pg, "select 1 as int");
//
//
//pg_wait($pg, function ($arg) {
//    var_dump(pg_fetch_all($arg));
// echo "hello";
//});

//var_dump($pg);

//mysqli_query($db, "select 1", MYSQLI_ASYNC | MYSQLI_STORE_RESULT);
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
//     file_get_contents_async("./stubs/serv.php1", $resolve);
// });
// //
// $timerId->then(function ($data) {
//     var_dump($data);
// //    var_dump($timerId);
// });

//  file_put_contents_async("try.php","dat1212a", fn()=>var_dump(1234));
// } catch(Exception $e){
// var_dump($e);
// }


// Promise::resolve(1)->then(function($i){
//                             var_dump($i);
//                             });
// Promise::reject("frsrgrgrrv")->catch(function($i){
// var_dump($i);
// });
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
//$mysql = mysqla_query(123);

// idle(function () {
//    echo "idle111";
// });
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
// print_r($db);
echo "sync exec ended.\n\n";

