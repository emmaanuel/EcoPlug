

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
        

        rooms_names = [
        {n:6, name:"Garage"},
        {n:2, name:"Etage"},
        {n:3, name:"Salon"},
        {n:4, name:"Jardin"}];

        
        $scope.motions = [];



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

        getParamMotion = function() {
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/params/motion'})
            .success(function(data, status) {
                if (data.params[0].value == "enabled") {
                    $scope.params_motion = true;
                } else {
                    $scope.params_motion = false;
                }
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };

        getMotion = function() {
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/motion/last'})
            .success(function(data, status) {
                $scope.motions = Array();
                data.motion.forEach(function(element, index, array){
                    $scope.motions.push({n:getRoomsName(element.n),d:element.d});
                });
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };

    

        minuteIntervalFunction = function(){
            $timeout(function() {
                getMotion();
                minuteIntervalFunction();
            }, 60000)
        };    
        minuteIntervalFunction();
        getMotion();
        getParamMotion();


        $scope.$watch('params_motion', function(newValue, oldValue) {
            newStatus = '';
            if (newValue == true){
                newStatus='enabled';
            } else {
                newStatus='disabled';
            }
            console.log('{value:"' + newStatus + '"}');
            $http.put('http://domo.emmaanuel.com/api/params/motion', '{"value":"' + newStatus + '"}')
            .error(function(data, status) {
                alert("Error");
            });
        });

        

    }]);



})();
