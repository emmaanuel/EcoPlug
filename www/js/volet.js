

//Loads the correct sidebar on window load,
//collapses the sidebar on window resize.
// Sets the min-height of #page-wrapper to window size
$(function() {
    $(window).bind("load resize", function() {
        topOffset = 50;
        width = (this.window.innerWidth > 0) ? this.window.innerWidth : this.screen.width;
        if (width < 768) {
            $('div.navbar-collapse').addClass('collapse');
            topOffset = 100; // 2-row-menu
        } else {
            $('div.navbar-collapse').removeClass('collapse');
        }

        height = ((this.window.innerHeight > 0) ? this.window.innerHeight : this.screen.height) - 1;
        height = height - topOffset;
        if (height < 1) height = 1;
        if (height > topOffset) {
            $("#page-wrapper").css("min-height", (height) + "px");
        }
    });

    var url = window.location;
    var element = $('ul.nav a').filter(function() {
        return this.href == url || url.href.indexOf(this.href) == 0;
    }).addClass('active').parent().parent().addClass('in').parent();
    if (element.is('li')) {
        element.addClass('active');
    }
});

(function() {

    var app = angular.module('domo',['frapontillo.bootstrap-switch']);

    app.controller('Dashboard',['$scope','$window','$timeout','$http','$log',function($scope,$window,$timeout,$http, $log){

        getRoomsName = function(n) {
            name = "Inconnu";
            rooms_names.forEach(function(element, index, array){
                if (n == element.n){
                    name = element.name;
                    return;
                }
            })
            return name;
        }

        getAction = function() {
            $http({method : 'GET',url : '/api/action/last'})
            .success(function(data, status) {
                $scope.actions = Array();
                data.action.forEach(function(element, index, array){
                    $scope.actions.push({d:element.date,a:element.action,p:element.processed});
                });
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };

        minuteIntervalFunction = function(){
            $timeout(function() {
                getAction();
                minuteIntervalFunction();
            }, 60000)
        };    
        minuteIntervalFunction();
        getAction();

        $scope.voletUp = function() {
            $http.post('/api/action', '{"a":"STORE_OPEN|"}')
            .error(function(data, status) {
                alert("Erreur lors de l'envoi de la commande");
            });
        }

        $scope.voletDown = function() {
            $http.post('/api/action', '{"a":"STORE_CLOSE|"}')
            .error(function(data, status) {
                alert("Erreur lors de l'envoi de la commande");
            });
        }

    }]);



})();
