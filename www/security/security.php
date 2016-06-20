<?php
include '../api/db.php';
$connected = false;
session_start();

if (isset($_SESSION['connected']) && $_SESSION['connected'] != '') {
	$connected = true;
}

if (isset($_SERVER['REMOTE_ADDR']) && $_SERVER['REMOTE_ADDR']=="XX.XX.XX.XX"){
	$connected = true;
}

if (isset($_COOKIE["token"])){
	$db = getDB();
    $sql = "Select user from domo_user" . getDBSuffix() . " where token=:token";
    $stmt = $db->prepare($sql);
    $stmt->bindParam("token", $_COOKIE["token"]);
    $stmt->execute();
    $r = $stmt->fetchAll(PDO::FETCH_OBJ);
    if (sizeof($r)>0){
    	$connected = true;
    }
}

if (!connected){	
	header ("Location: /pages/login.php");
	exit();
}
?>