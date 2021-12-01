<?php
$serv1 = new HttpServer(81, "tcp://0.0.0.0");
$serv1->setPublicPath(__DIR__);
$serv1->on_request(function (HttpRequest $req, HttpResponse $res) {
//    var_dump($req, $res, $this->publicPath);
    if (file_exists($this->publicPath.$req->uri) && !is_dir($this->publicPath.$req->uri)){
        echo  "WTF???";
        $result = new finfo();
        $mimeType = $result->file($this->publicPath.$req->uri, FILEINFO_MIME_TYPE);
       $file ="1";//file_get_contents($this->publicPath.$req->uri);
        echo "1234 $this->publicPath.$req->uri\n";


      $res->setHeader("Content-Type", "mimeType")
           ->setHeader("Content-Length", strlen($file));
       $res->send($file);
       return;
    }
    $res->setHeader("Content-Type", "text/plain; charset=utf-8");
    $res->send("hello\n");

});
