

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
        $scope.g = new JustGage({
            id: "gauge",
            value: 50,
            min: 0,
            max: 6000,
            label: "Watts",
            title: "Consommation Electrique"
        });

        $scope.rooms = [
        {node: "2", name:"Etage", color: "primary", icon: "fa-bed", temp:"22,5", rh:"45", date:"", light:null},
        {node: "3", name:"Salon", color: "green", icon: "fa-coffee", temp:"20", rh:"46", date:"", light:null},
        {node: "4", name:"Jardin", color: "yellow", icon: "fa-tree", temp:"", rh:"", date:"", light:"0"},
        {node: "5", name:"Garage", color: "red", icon: "fa-car", temp:"22,5", rh:null, date:"", light:null},
        {node: "6", name:"RDC", color: "info", icon: "fa-home", temp:"22,5", rh:null, date:"", light:null}];

        $scope.lastEDFupdate = ".";
        $scope.HP="";
        $scope.HC="";
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


        getrooms = function() {
            $http({method : 'GET',url : '/api/temp/last'})

            .success(function(data, status) {
                data.temp.forEach(function(element, index, array){
                    updateRoom(element.d, element.n, element.t, element.h, element.l)
                });
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };

        getEDF = function() {
            $http({method : 'GET',url : '/api/edf/last'})
            .success(function(data, status) {
                data.edf.forEach(function(element, index, array){
                    $scope.g.refresh(element.pw);
                    $scope.lastEDFupdate = element.d;
                    $scope.HP = element.hp;
                    $scope.HC = element.hc;
                });
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };


        updateRoom = function(d, n, temp, rh, l){
            $scope.rooms.forEach(function(element, index, array){
                if (element.node == n){
                    element.temp = temp;
                    element.rh = rh;
                    element.date = d;
                    element.light = l;
                }
            });
        }

        minuteIntervalFunction = function(){
            $timeout(function() {
                getrooms();
                if ($scope.graph) {
                    $scope.graph.redraw();
                }
                
                minuteIntervalFunction();
            }, 60000)
        };    
        minuteIntervalFunction();
        getrooms();

        secondIntervalFunction = function(){
            $timeout(function() {
                getEDF();
                secondIntervalFunction();
            }, 1000)
        };    
        secondIntervalFunction();



    }]);



})();
