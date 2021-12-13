<?php
/ $serv1 = new Server(8200, callback: fn() => 1);
 $str = "HTTP/1.1 200 OK\n\r";
 $serv1->on_data(function ($data) use (&$serv1) {
     if ($data == "get_cool")
          $serv1->write("cool");
     var_dump("on_data8200: " . $data);

 //                $serv1->write(" from php hello");
 });
 $serv1->write("from php hello");
 $serv1->write($str);
 var_dump($serv1);