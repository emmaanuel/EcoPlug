<?php
include 'db.php';
include '../security/security.php';
require 'Slim/Slim.php';
\Slim\Slim::registerAutoloader();

$app = new \Slim\Slim(array('mode' => 'development', 'debug' => true, 'log.level' => \Slim\Log::DEBUG, 'log.enabled' => true));

$app->get('/temp/date/:date/node/:node','getTempbyDateNode');
$app->get('/temp/day/','getTempLastDay');
$app->get('/temp/week/','getTempLastWeek');
$app->get('/temp/month/','getTempLastMonth');
$app->get('/temp/last','getLastTemp');
$app->post('/temp','insertTemp');

$app->get('/edf/date/:date','getEdfbyDate');
$app->get('/edf/last','getLastEdf');
$app->get('/edf/week','getEdfLastWeek');
$app->post('/edf','insertEdf');

$app->post('/gaz','insertGaz');

$app->post('/metrics','insertMetrics');

$app->post('/motion', 'insertMotion');
$app->get('/motion/last', 'getLastMotion');

$app->post('/log', 'insertLog');
$app->get('/log/last', 'getLastLog');

$app->get('/params/:name', 'getParams');
$app->get('/params', 'getAllParams');
$app->put('/params/:name', 'setParams');

$app->get('/action/next','getNextAction');
$app->get('/action/last','getLastAction');
$app->post('/action/process','processAction');
$app->post('/action','insertAction');
$app->post('/action/log','logAction');

$app->post('/heater','logHeater');

$app->get('/bad','getBadInfo');
$app->post('/bad/:id/speed/:speed','insertBadScore');
$app->delete('/bad','resetBad');

$app->run();

function getTempbyDateNode($date, $node) {
	try {
		$db = getDB();
		$sql = "select DATE_FORMAT(date, '%y-%m-%d %H:%i') as d, node as n, round(temp,1) as t, round(rh,1) as h , light as l from domo_temp" . getDBSuffix() . " where date(date)=:date and node =:node order by date";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("date", $date);
		$stmt->bindParam("node", $node);
		$stmt->execute();
		$temp = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"temp": ' . json_encode($temp) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getTempLastDay() {
	try {
		$db = getDB();
		$sql = "select ROUND(UNIX_TIMESTAMP(date)/(15 * 60))*15*60 as d, node as n, round(temp,1) as t, round(rh,1) as h , light as l from domo_temp" . getDBSuffix() . " where UNIX_TIMESTAMP(date)>=(UNIX_TIMESTAMP(now())-(3600*24)) group by d,n order by date";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("date", $date);
		$stmt->bindParam("node", $node);
		$stmt->execute();
		$temp = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"temp": ' . json_encode($temp) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getTempLastWeek() {
	try {
		$db = getDB();
		$sql = "select ROUND(UNIX_TIMESTAMP(date)/(3600))*3600 as d, node as n, round(temp,1) as t, round(rh,1) as h , light as l from domo_temp" . getDBSuffix() . " where UNIX_TIMESTAMP(date)>=(UNIX_TIMESTAMP(now())-(3600*24*7)) group by d,n order by date";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("date", $date);
		$stmt->bindParam("node", $node);
		$stmt->execute();
		$temp = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"temp": ' . json_encode($temp) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getTempLastMonth() {
	try {
		$db = getDB();
		$sql = "select ROUND(UNIX_TIMESTAMP(date)/(3600))*3600 as d, node as n, round(temp,1) as t, round(rh,1) as h , light as l from domo_temp" . getDBSuffix() . " where UNIX_TIMESTAMP(date)>=(UNIX_TIMESTAMP(now())-(3600*24*7*30)) group by d,n order by date";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("date", $date);
		$stmt->bindParam("node", $node);
		$stmt->execute();
		$temp = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"temp": ' . json_encode($temp) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getLastTemp() {
	try {
		$db = getDB();
		$sql = "select d,n,t,h,l from (select DATE_FORMAT(date, '%y-%m-%d %H:%i') as d, node as n , round(temp,1) as t, round(rh,1) as h, light as l from domo_temp" . getDBSuffix() . " order by d desc) as table1 group by n";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$temp = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"temp": ' . json_encode($temp) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function insertTemp() {
	$request = \Slim\Slim::getInstance()->request();
	$temp = json_decode($request->getBody());
	$t = (($temp->t == "")?null:$temp->t);
	$h = (($temp->h == "")?null:$temp->h);
	$l = (($temp->l=="")?null:$temp->l);
	$sql = "INSERT INTO domo_temp" . getDBSuffix() . " VALUES(NOW(),:node,:temp,:humi,:rx,:light)";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("node", $temp->n );
		$stmt->bindParam("temp", $t );
		$stmt->bindParam("humi", $h );
		$stmt->bindParam("rx", $temp->r );
		$stmt->bindParam("light", $l);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 

	}
}

function getEdfbyDate($date) {
	try {
		$db = getDB();
		$sql = "select DATE_FORMAT(date, '%y-%m-%d %H:%i') as d, power as r from domo_edf" . getDBSuffix() . " where date(date)=:date";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("date", $date);
		$stmt->execute();
		$edf = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"edf": ' . json_encode($edf) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getLastEdf() {
	try {
		$db = getDB();
		$sql = "Select truncate((max(hp)-min(hp))*0.152868/1000+(max(hc)-min(hc))*0.1105588/1000,2) as euros from domo_edf" . getDBSuffix() . " where date(date)=CURDATE()";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$prix = $stmt->fetchAll(PDO::FETCH_OBJ);
		$euros = $edf[0]->euros;
		if ($euros == null) $euros=0;
		$sql = "SELECT DATE_FORMAT(date, '%y-%m-%d %H:%i:%s') as d, conso as pw,truncate(hc/1000,0) as hc ,truncate(hp/1000,0) as hp, " . $euros . " as euros FROM domo_edf_live" . getDBSuffix() . " order by date desc limit 1";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$edf = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"edf": ' . json_encode($edf) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getEdfLastWeek() {
	try {
		$db = getDB();
		/*
		$sql = "Select truncate((max(hp)-min(hp))*0.152868/1000+(max(hc)-min(hc))*0.1105588/1000,2) as euros from domo_edf where UNIX_TIMESTAMP(date)>=(UNIX_TIMESTAMP(now())-(3600*24*7))";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$prix = $stmt->fetchAll(PDO::FETCH_OBJ);
		$euros = $edf[0]->euros;
		if ($euros == null) $euros=0;
		*/
		$sql = "SELECT round(UNIX_TIMESTAMP(date)/(3600))*3600 as d, max(power,0) as pw, min(truncate(hc/1000,0)) as hc , min(truncate(hp/1000,0)) as hp FROM domo_edf" . getDBSuffix() . " where UNIX_TIMESTAMP(date)>=(UNIX_TIMESTAMP(now())-(3600*24*7)) group by d order by date desc ";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$edf = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"edf": ' . json_encode($edf) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function insertEdf() {
	$request = \Slim\Slim::getInstance()->request();
	$temp = json_decode($request->getBody());
	$sql = "INSERT INTO domo_edf_live" . getDBSuffix() . " VALUES(NOW(),:power,:tf,:hc,:hp)";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("power", $temp->pw);
		$stmt->bindParam("tf", $temp->tf);
		$stmt->bindParam("hc", $temp->hc);
		$stmt->bindParam("hp", $temp->hp);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function insertGaz() {
	$request = \Slim\Slim::getInstance()->request();
	$gaz = json_decode($request->getBody());
	$sql = "INSERT INTO domo_gaz" . getDBSuffix() . " VALUES(NOW(),:pulse)";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("pulse", $gaz->pulse);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}


function insertMetrics() {
	$request = \Slim\Slim::getInstance()->request();
	$metrics = json_decode($request->getBody());
	print("Metrics:");
	print_r($metrics);
	
	foreach ($metrics as $metric){
		try {
			print_r($metric);
			$sql = "INSERT INTO domo_metrics" . getDBSuffix() . " VALUES(:timestamp,:metric,:value)";
			$db = getDB();
			$stmt = $db->prepare($sql);
			$stmt->bindParam("timestamp", $metric->timestamp);
			$stmt->bindParam("metric", $metric->metric);
			$stmt->bindParam("value", $metric->value);
			$stmt->execute();
			$db = null;
		} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
			echo '{"error":{"text":'. $e->getMessage() .'}}'; 
		}
	}

}

function insertMotion() {
	$request = \Slim\Slim::getInstance()->request();
	$motion = json_decode($request->getBody());
	
	try {
		$db = getDB();
		$sql = "SELECT value from domo_params" . getDBSuffix() . " where name='motion'";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$result = $stmt->fetch(PDO::FETCH_ASSOC);
		if ($result["value"] == "enabled") {
			$sql = "INSERT INTO domo_motion" . getDBSuffix() . " VALUES(NOW(),:node,:rx)";
			$stmt = $db->prepare($sql);
			$stmt->bindParam("node", $motion->n);
			$stmt->bindParam("rx", $motion->r);
			$stmt->execute();  
			$headers ='From: votreemail@email.com'."\n";
			$headers .='Reply-To: votreemeail@email.com'."\n";
			$headers .='Content-Type: text/plain; charset="iso-8859-1"'."\n";
			$headers .='Content-Transfer-Encoding: 8bit';
			mail('trigger@recipe.ifttt.com', 'Detection de Mouvement', 'Message contenu de l email', $headers); 
		}
		
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getLastMotion() {
	try {
		$db = getDB();
		$sql = "Select node as n, date as d from domo_motion" . getDBSuffix() . " ORDER BY date DESC LIMIT 5";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$motion = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"motion": ' . json_encode($motion) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function insertLog() {
	$request = \Slim\Slim::getInstance()->request();
	echo $request->getBody();
	$log = json_decode($request->getBody());
	try {
		$db = getDB();
		$sql = "INSERT INTO domo_logs" . getDBSuffix() . " VALUES(NOW(),:node,:msg)";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("node", $log->n);
		$stmt->bindParam("msg", $log->msg);
		echo "node:" . $log->n;
		$stmt->execute();
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getLastLog() {
	try {
		$db = getDB();
		$sql = "Select node as n, date as d, message as m from domo_logs" . getDBSuffix() . " ORDER BY date DESC LIMIT 50";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$logs = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"logs": ' . json_encode($logs) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getParams($name) {
	try {
		$db = getDB();
		$sql = "Select * from domo_params" . getDBSuffix() . " where name=:name";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("name", $name);
		$stmt->execute();
		$param = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"params": ' . json_encode($param) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function setParams($name) {
	try {
		$request = \Slim\Slim::getInstance()->request();
		$param = json_decode($request->getBody());
		$db = getDB();
		$sql = "update domo_params" . getDBSuffix() . " set value=:value where name=:name";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("name", $name);
		$stmt->bindParam("value", $param->value);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getAllParams() {
	try {
		$db = getDB();
		$sql = "Select * from domo_params" . getDBSuffix() . "";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$param = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"params": ' . json_encode($param) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getNextAction() {
	try {
		$db = getDB();
		$sql = "Select * from domo_actions" . getDBSuffix() . " where processed=0 ORDER BY date ASC LIMIT 1";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$action = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"action": ' . json_encode($action) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getLastAction() {
	try {
		$db = getDB();
		$sql = "Select date, action, processed from domo_actions" . getDBSuffix() . " ORDER BY date DESC LIMIT 5";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$action = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"action": ' . json_encode($action) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function processAction() {
	try {
		$request = \Slim\Slim::getInstance()->request();
		$action = json_decode($request->getBody());
		$db = getDB();
		$sql = "update domo_actions" . getDBSuffix() . " set processed=TRUE where id=:id";
		$stmt = $db->prepare($sql);
		$stmt->bindParam("id", $action->id);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function insertAction() {
	$request = \Slim\Slim::getInstance()->request();
	$action = json_decode($request->getBody());
	$sql = "INSERT INTO domo_actions" . getDBSuffix() . " VALUES(null, NOW(),:action,0)";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("action", $action->a);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function logAction() {
	$request = \Slim\Slim::getInstance()->request();
	$action = json_decode($request->getBody());
	$sql = "INSERT INTO domo_actions" . getDBSuffix() . " VALUES(null, NOW(),:action,1)";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("action", $action->a);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function logHeater() {
	$request = \Slim\Slim::getInstance()->request();
	$event = json_decode($request->getBody());
	$sql = "INSERT INTO domo_heater_evt" . getDBSuffix() . " VALUES(null, NOW(),:event)";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("event", $event->s);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function getBadInfo() {
	try {
		$db = getDB();
		$sql = "select * from domo_bad order by playerid";
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$temp = $stmt->fetchAll(PDO::FETCH_OBJ);
		$db = null;
		echo '{"players": ' . json_encode($temp) . '}';
	} catch(PDOException $e) {
	    //error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}

function insertBadScore($id, $speed) {
	if ($speed<125){
		$sql = "update domo_bad" . getDBSuffix() . " set score=score+:speed, speed=:speed where playerid=:playerid";
	} else {
		$sql = "update domo_bad" . getDBSuffix() . " set speed=:speed where playerid=:playerid";
	}
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->bindParam("speed", $speed );
		$stmt->bindParam("playerid", $id );
		$stmt->execute();
		if ($stmt->affected_rows == 0){
			$stmt = $db->prepare("insert into domo_bad" . getDBSuffix() . " values (:playerid,:speed,:speed)");
			$stmt->bindParam("playerid", $id );
			$stmt->bindParam("speed", $speed );
			$stmt->execute();
		}
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
	
}

function resetBad() {
	$sql = "truncate table domo_bad" . getDBSuffix() . "";
	try {
		$db = getDB();
		$stmt = $db->prepare($sql);
		$stmt->execute();
		$db = null;
	} catch(PDOException $e) {
		//error_log($e->getMessage(), 3, '/var/tmp/php.log');
		echo '{"error":{"text":'. $e->getMessage() .'}}'; 
	}
}


?>