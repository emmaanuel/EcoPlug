<?php
session_start();
if (!(isset($_SESSION['connected']) && $_SESSION['connected'] != '')) {
	if (!(isset($_SERVER['REMOTE_ADDR']) && $_SERVER['REMOTE_ADDR']=="XX.XX.XX.XX")){
		header ("Location: /pages/login.php");
		exit();
	}
}
?>