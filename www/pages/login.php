<?PHP
include '../api/db.php';
$login = "";
$pass = "";

if (isset($_GET['logout'])){
    session_start();
    unset($_SESSION['connected']);
}

if ((isset($_POST['inputlogin']) && $_POST['inputlogin'] != '')) {
    $login = $_POST['inputlogin'];
}
if ((isset($_POST['inputPassword']) && $_POST['inputPassword'] != '')) {
    $pass = $_POST['inputPassword'];
}
if (($login != "") && ($pass != "")){
    $h = hash("sha256",$pass);
    $db = getDB();
    $sql = "Select user, token from domo_user" . getDBSuffix() . " where user=:user and hash=:hash";
    $stmt = $db->prepare($sql);
    $stmt->bindParam("user", $login);
    $stmt->bindParam("hash", $h);
    $stmt->execute();
    $r = $stmt->fetchAll(PDO::FETCH_OBJ);
    if (sizeof($r)>0){
        session_start();
        $_SESSION['connected']="YES";
        header ("Location: /");
        $uuid = uniqid();
        $sql = "update domo_user" . getDBSuffix() . " set token=:uuid where user=:user";
        $stmt = $db->prepare($sql);
        $stmt->bindParam("user", $login);
        $stmt->bindParam("uuid", $uuid);
        $stmt->execute();
        setcookie("token",$uuid,time()+60*60*24*365);
        exit();
    } else {
        $error = "Invalid credentials";
    }
}


?>   
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">
    <title>Domo v4 - login</title>
    <link href="../bower_components/bootstrap/css/bootstrap.min.css" rel="stylesheet">
    <link href="../bower_components/metisMenu/metisMenu.min.css" rel="stylesheet">
    <link href="../css/signin.css" rel="stylesheet">
    <link href="../bower_components/bootstrap-switch/bootstrap-switch.min.css" rel="stylesheet">
    <link rel="stylesheet" href="//maxcdn.bootstrapcdn.com/font-awesome/4.3.0/css/font-awesome.min.css">
</head>
<body>
 <div class="container">
  <form class="form-signin" method="POST">
    <?PHP
    if (isset($error)){
    ?>
    <div class="alert alert-danger" role="alert"><?=$error?></div>
    <?PHP
    }
    ?>
    <h2 class="form-signin-heading">Please sign in</h2>
    <label for="inputlogin" class="sr-only">login</label>
    <input type="input" name="inputlogin" id="inputlogin" class="form-control" placeholder="Login" required autofocus>
    <label for="inputPassword" class="sr-only">Password</label>
    <input type="password" name="inputPassword" id="inputPassword" class="form-control" placeholder="Password" required>
    <div class="checkbox">
      <label>
        
    </label>
</div>
<button class="btn btn-lg btn-primary btn-block" type="submit">Sign in</button>
</form>
</div> 
<script src="../bower_components/jquery/jquery.min.js"></script>
<script src="../bower_components/bootstrap/js/bootstrap.min.js"></script>
<script src="../bower_components/raphael/raphael-min.js"></script>
<script src="../bower_components/metisMenu/metisMenu.min.js"></script>
<script src="../bower_components/justgage/justgage.1.0.1.min.js"></script>
<script src="../bower_components/bootstrap-switch/bootstrap-switch.min.js"></script>
<script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.3.11/angular.js"></script>
<script src="../bower_components/angular-bootstrap-switch/angular-bootstrap-switch.min.js"></script>
<script src="../js/sb-admin-2.js"></script>
</body>
</html>
