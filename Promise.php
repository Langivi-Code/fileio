<?php

// class Promise
// {
//
//     private $dataStore;
//     private Fiber $internalFiber;
//     private $status = PromiseStatus::Pending;
//     private bool $promiseFinalised = false;
//
//     /**
//      * Promise constructor.
//      */
//     public function __construct(private \Closure $closure)
//     {
//  $this->handle();
// //        $this->internalFiber = new Fiber([$this, 'handle']);
// //        $this->status = $this->internalFiber->start();
//     }
//
//     public static function resolve($data)
//     {
//         return new Promise(function ($res, $rej) use ($data) {
//             $res($data);
//         });
//     }
//
//     public static function reject($data)
//     {
//         return new Promise(function ($res, $rej) use ($data) {
//             $rej($data);
//         });
//     }
//
//     private function _then(callable $handle)
//     {
//         if ($this->status == PromiseStatus::Resolved && !$this->promiseFinalised) {
// //            $this->internalFiber->resume();
//             $retval = $handle($this->dataStore);
//             $this->dataStore = null;
//             return $retval instanceof Promise ? $retval : $this;
//         }
//         else{
//             $this->then($handle);
//         }
//     }
//     public function then(callable $handler) {
//         $then = $this->toCallback('_then');
//         idle(fn ()=>$then($handler));
//     }
//
//     public function catch(callable $handler)
//     {
//         if ($this->status == PromiseStatus::Rejected && !$this->promiseFinalised) {
// //            $this->internalFiber->resume();
//             $handler($this->dataStore);
//         }
//
//     }
//
//     public function finally(callable $handler)
//     {
//         if ($this->status != PromiseStatus::Pending && !$this->promiseFinalised) {
//             $handler();
//             $this->promiseFinalised = true;
//         }
//     }
//
//     private function _resolve($data)
//     {
// //        Fiber::suspend(PromiseStatus::Resolved);
//         $this->status=PromiseStatus::Resolved;
//         $this->dataStore = $data;
//
//     }
//
//     private function _reject($data)
//     {
// //        Fiber::suspend(PromiseStatus::Rejected);
//         $this->dataStore = $data;
//     }
//
//     private function toCallback($function)
//     {
//         $fn = fn($arg) => $this->$function($arg);
//
//         return $fn->bindTo($this);
//     }
//
//     private function handle()
//     {
//         try {
//             $closure = $this->closure;
//             $closure($this->toCallback('_resolve'), $this->toCallback('_reject'));
//
//         } catch (\Exception $exception) {
//             $this->reject($exception);
// //            $resumedHandler = Fiber::suspend();
//         }
//
//     }
// }
