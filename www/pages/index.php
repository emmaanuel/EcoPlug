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
    <title>Domo v4.0</title>
    <link href="../bower_components/bootstrap/css/bootstrap.min.css" rel="stylesheet">
    <link href="../bower_components/metisMenu/metisMenu.min.css" rel="stylesheet">
    <link href="../css/sb-admin-2.css" rel="stylesheet">
    <link href="../bower_components/bootstrap-switch/bootstrap-switch.min.css" rel="stylesheet">
    <link rel="stylesheet" href="//maxcdn.bootstrapcdn.com/font-awesome/4.3.0/css/font-awesome.min.css">
</head>
<body>
    <div id="wrapper">
        <nav class="navbar navbar-default navbar-static-top" role="navigation" style="margin-bottom: 0">
            <div class="navbar-header">
                <a href="/" class="navbar-brand">Domo v4.0</a>
            </div>
        </nav>
        <div id="page-wrapper" ng-controller="Dashboard as dash">
            <div class="row">
                <div class="col-lg-12">
                    <h1 class="page-header">Tableau de bord</h1>
                </div>
            </div>
            <div class="row">
                <div class="col-lg-3 col-md-6" ng-repeat="room in rooms">
                    <div class="panel panel-{{room.color}}">
                        <div class="panel-heading">
                            <div class="row">
                                <div class="col-xs-3">
                                    <i class="fa {{room.icon}} fa-5x"></i>
                                    <div ng-cloak>{{room.name}}</div>
                                </div>
                                <div class="col-xs-3 text-center">
                                    <div ng-hide="(room.light == null) || room.light >50 "><i class="fa fa-moon-o fa-5x" alt="{{room.light}}"></i><div>Nuit</div></div>
                                    <span ng-hide="(room.light == null) || room.light <=50 "><i class="fa fa-sun-o fa-5x" alt="{{room.light}}"></i><div>Jour</div></span>
                                </div>
                                <div class="col-xs-6 text-right" ng-cloak>
                                    <span class="huge"> {{room.temp}}°C</span>
                                    <div ng-hide="(room.rh == null)">{{room.rh}}% humidité</div>
                                    <div class="verysmall">{{room.date}}</div>
                                </div>
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
        <div class="row">
            <div class="col-lg-12 col-md-12">
                <div class="panel panel-default">
                    <div class="panel-heading">
                        <i class="fa fa-bell fa-fw"></i>Détection de mouvements
                        <span class="pull-right text-muted small"><input bs-switch type="checkbox" name="motion-checkbox" data-size="mini" ng-model="params_motion"></span>
                    </div>
                    <div class="panel-body">
                        <div class="list-group">
                            <div  class="list-group-item" ng-repeat="motion in motions">
                                <i class="fa fa-coffee fa-fw" ng-cloak></i> {{motion.n}}
                                <span class="pull-right text-muted small" ng-cloak><em>{{motion.d}}</em>
                                </span>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="row">
            <div class="col-lg-12 col-md-12">
                <div class="panel panel-default">
                    <div class="panel-heading">
                        <i class="fa fa-arrows-v fa-fw"></i>Contrôle des volets
                        <span class="pull-right text-muted small"></span>
                    </div>
                    <div class="panel-body">
                        <div class="row ">
                            <div class="col-lg-12 col-md-12">
                                <div class="row">
                                    <div class="text-center">
                                        <i class="fa fa-caret-square-o-up fa-5x" ng-click="voletUp()"></i>
                                    </div>
                                </div>
                                <div class="row">
                                    <div class="text-center">
                                        <i class="fa fa-caret-square-o-down fa-5x" ng-click="voletDown()"></i>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="row">
            <div class="col-lg-12 col-md-12">
                <div class="panel panel-default">
                    <div class="panel-heading">
                        <i class="fa fa-check-circle-o fa-fw"></i>Historique des Actions
                    </div>
                    <div class="panel-body">
                        <table class="table table-striped">
                            <tr ng-repeat="action in actions">
                                <td>{{action.d}}</td>
                                <td>{{action.a}}</td>
                                <td>
                                    <div ng-if="action.p==1">
                                        <i class="fa fa-check-circle-o fa-2x"></i>
                                    </div>
                                    <div ng-if="action.p==0">
                                        <i class="fa fa-clock-o fa-2x"></i>
                                    </div>
                                </td>
                            </tr>
                        </table>
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
<script src="../bower_components/justgage/justgage.1.0.1.min.js"></script>
<script src="../bower_components/bootstrap-switch/bootstrap-switch.min.js"></script>
<script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.3.11/angular.js"></script>
<script src="../bower_components/angular-bootstrap-switch/angular-bootstrap-switch.min.js"></script>
<script src="../js/sb-admin-2.js"></script>
</body>
</html>
