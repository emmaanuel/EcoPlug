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
                    </ul>
                </div>
                <!-- /.sidebar-collapse -->
            </div>
            <!-- /.navbar-static-side -->
        </nav>
        <div id="page-wrapper" ng-controller="Dashboard as dash">
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
                                        <button type="button" class="btn btn-default" ng-click="voletUp()">
                                        <i class="fa fa-caret-square-o-up fa-5x"></i>
                                        </button>
                                    </div>
                                </div>
                                <br>
                                <div class="row">
                                    <div class="text-center">
                                        <button type="button" class="btn btn-default" ng-click="voletDown()">
                                        <i class="fa fa-caret-square-o-down fa-5x"></i>
                                        </button>
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
<script src="../js/volet.js"></script>
</body>
</html>
