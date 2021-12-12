<?php

 class Promise
 {
     private $dataStore;
     private PromiseStatus $status = PromiseStatus::Pending;
     private bool $promiseFinalised = false;

     /**
      * Promise constructor.
      */
     public function __construct(private \Closure $closure) {}
     public static function resolve($data):Promise{}
     public static function reject($data):Promise{}
     public function then(callable $handler):Promise | mixed {}
     public function catch(callable $handler):Promise | mixed {}

     private function _then(callable $handle)
     {
         if ($this->status == PromiseStatus::Resolved && !$this->promiseFinalised) {
 //            $this->internalFiber->resume();
             $retval = $handle($this->dataStore);
             $this->dataStore = null;
             return $retval instanceof Promise ? $retval : $this;
         }
         else{
             $this->then($handle);
         }
     }
     public function finally(callable $handler): void
     {
         if ($this->status != PromiseStatus::Pending && !$this->promiseFinalised) {
             $handler();
             $this->promiseFinalised = true;
         }
     }
 }
