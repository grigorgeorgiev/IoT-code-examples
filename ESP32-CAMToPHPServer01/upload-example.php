<?php

$received = file_get_contents('php://input');
$fileToWrite = "timelapse/upload - ".time().".jpg";
file_put_contents($fileToWrite, $received);
