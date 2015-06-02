(function() {

    $(window).bind("load resize", function() {
        topOffset = 50;
        height = ((this.window.innerHeight > 0) ? this.window.innerHeight : this.screen.height) - 1;
        height = height - topOffset;
        if (height < 1) height = 1;
        if (height > topOffset) {
            $("#page-wrapper").css("min-height", (height) + "px");
        }
    });

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

        rooms_names = [
        {n:1, name:"Garage"},
        {n:2, name:"Etage"},
        {n:3, name:"Salon"},
        {n:4, name:"Jardin"}];

        $scope.rooms = [
        {node: "2", name:"Etage", color: "primary", icon: "fa-bed", temp:"22,5", rh:"45", date:"", light:null},
        {node: "3", name:"Salon", color: "green", icon: "fa-coffee", temp:"20", rh:"46", date:"", light:null},
        {node: "4", name:"Jardin", color: "yellow", icon: "fa-tree", temp:"", rh:"", date:"", light:"0"},
        {node: "1", name:"Garage", color: "red", icon: "fa-car", temp:"22,5", rh:null, date:"", light:null}];
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
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/temp/last', headers: { 'X-Parse-Application-Id':'XXX', 'X-Parse-REST-API-Key':'YYY'}})
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
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/edf/last', headers: { 'X-Parse-Application-Id':'XXX', 'X-Parse-REST-API-Key':'YYY'}})
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

        getParamMotion = function() {
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/params/motion', headers: { 'X-Parse-Application-Id':'XXX', 'X-Parse-REST-API-Key':'YYY'}})
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
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/motion/last', headers: { 'X-Parse-Application-Id':'XXX', 'X-Parse-REST-API-Key':'YYY'}})
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
                getMotion();
                minuteIntervalFunction();
            }, 60000)
        };    
        minuteIntervalFunction();
        getrooms();
        getMotion();
        getParamMotion();

        secondIntervalFunction = function(){
            $timeout(function() {
                getEDF();
                secondIntervalFunction();
            }, 1000)
        };    
        secondIntervalFunction();

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

        $scope.items = ['item1', 'item2', 'item3'];



    }]);



})();
