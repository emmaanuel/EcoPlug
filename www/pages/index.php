<?PHP
include '../security/security.php';
?>
<!DOCTYPE html>
<html lang="fr" ng-app="domo">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">
    <title>Domo v5.0</title>
    <link href="../bower_components/bootstrap/css/bootstrap.min.css" rel="stylesheet">
    <link href="../bower_components/metisMenu/metisMenu.min.css" rel="stylesheet">
    <link href="../css/sb-admin-2.css" rel="stylesheet">
    <link href="../bower_components/bootstrap-switch/bootstrap-switch.min.css" rel="stylesheet">
    <link rel="stylesheet" href="//maxcdn.bootstrapcdn.com/font-awesome/4.3.0/css/font-awesome.min.css">
</head>
<body>
    <div id="wrapper">
        <!-- Navigation -->
        <nav class="navbar navbar-default navbar-static-top" role="navigation" style="margin-bottom: 0">
            <div class="navbar-header">
                <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
                    <span class="sr-only">Toggle navigation</span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                </button>
                <a class="navbar-brand" href="index.html">Domo v5.0</a>
            </div>
            <!-- /.navbar-header -->

            <div class="navbar-default sidebar" role="navigation">
                <div class="sidebar-nav navbar-collapse">
                    <ul class="nav" id="side-menu">
                        <li>
                            <a href="index.php"><i class="fa fa-dashboard fa-fw"></i> Dashboard</a>
                        </li>
                        <li>
                            <a href="volet.php"><i class="fa fa-table fa-fw"></i> Volet</a>
                        </li>
                        <li>
                            <a href="motion.php"><i class="glyphicon glyphicon-sunglasses"></i> Mouvement</a>
                        </li>
                        <li>
                            <a href="thermo.php"><i class="glyphicon glyphicon-leaf"></i> Thermostat</a>
                        </li>
                        <li>
                            <a href="bad.php"><i class="glyphicon glyphicon-flag"></i> Brosse a dents</a>
                        </li>
                    </ul>
                </div>
                <!-- /.sidebar-collapse -->
            </div>
            <!-- /.navbar-static-side -->
        </nav>
        <div id="page-wrapper" ng-controller="Dashboard as dash">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Tableau de bord</h1>
                </div>
            </div>
            <div class="row">
                <div class="col-lg-2 col-md-6" ng-repeat="room in rooms">
                    <div class="panel panel-{{room.color}}">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                    <i class="fa {{room.icon}} fa-4x"></i>
                                    <div ng-cloak>{{room.name}}</div>
                                </div>
                                <div class="col-xs-3 text-center">
                                    <div ng-hide="(room.light == null) || room.light >30 "><i class="fa fa-moon-o fa-3x" alt="{{room.light}}"></i><div>Nuit</div></div>
                                    <span ng-hide="(room.light == null) || room.light <=30 "><i class="fa fa-sun-o fa-3x" alt="{{room.light}}"></i><div>Jour</div></span>
                                </div>
                                <div class="col-xs-6 text-right" ng-cloak>
                                    <span class="huge"> {{room.temp}}°C</span>
                                    <div ng-hide="(room.rh == null)">{{room.rh}}% HR</div>
                                    
                                </div>
                            </div>
                            <div class="row text-center">
                                {{room.date}}
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="row">
                <div class="col-lg-12 col-md-12">
                    <div class="panel panel-default">
                        <div class="panel-heading">
                            EDF
                        </div>
                        <div class="panel-body">
                            <div class="row">
                                <div class="col-lg-6 col-md-6">
                                    <div id="gauge"></div>
                                </div>
                                <div class="col-lg-6 col-md-6">
                                 <table class="table">
                                    <tr>
                                        <td>Heures Pleines</td>
                                        <td ng-cloak>{{HP}}</td>
                                    </tr>
                                    <tr>
                                        <td>Heures Creuses</td>
                                        <td ng-cloak>{{HC}}</td>
                                    </tr>
                                </table>
                            </div>
                        </div>
                        <div class="verysmall text-center" ng-cloak>{{lastEDFupdate}}</div>
                    </div>
                </div>
            </div>
        </div>
    </div>
    <div class="text-center">
        <a href="/pages/login.php?logout=true"><i class="fa fa-unlock-alt fa-2x"></i></a>
    </div>
</div>
</div>
<script src="../bower_components/jquery/jquery.min.js"></script>
<script src="../bower_components/bootstrap/js/bootstrap.min.js"></script>
<script src="../bower_components/raphael/raphael-min.js"></script>
<script src="../bower_components/metisMenu/metisMenu.min.js"></script>
<script src="../bower_components/justgage/justgage-1.1.0.min.js"></script>
<script src="../bower_components/bootstrap-switch/bootstrap-switch.min.js"></script>
<script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.4.7/angular.js"></script>
<script src="../bower_components/angular-bootstrap-switch/angular-bootstrap-switch.min.js"></script>
<script src="../js/sb-admin-2.js"></script>
</body>
</html>
