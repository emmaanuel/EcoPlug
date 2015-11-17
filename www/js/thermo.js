

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
        
$scope.quantity1 = 20.5
        rooms_names = [
        {n:6, name:"Garage"},
        {n:2, name:"Etage"},
        {n:3, name:"Salon"},
        {n:4, name:"Jardin"}];

        $scope.rooms = [
        {node: "2", name:"Etage", color: "primary", icon: "fa-bed", temp:"22,5", rh:"45", date:"", light:null},
        {node: "3", name:"Salon", color: "green", icon: "fa-coffee", temp:"20", rh:"46", date:"", light:null},
        {node: "4", name:"Jardin", color: "yellow", icon: "fa-tree", temp:"", rh:"", date:"", light:"0"},
        {node: "6", name:"Garage", color: "red", icon: "fa-car", temp:"22,5", rh:null, date:"", light:null}];
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
            $http({method : 'GET',url : 'http://domo.emmaanuel.com/api/temp/last'})
            .success(function(data, status) {
                data.temp.forEach(function(element, index, array){
                    updateRoom(element.d, element.n, element.t, element.h, element.l)
                });
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };

        

    }]);

app.directive("counter", function() {
  return {
    restrict: "A",
    scope: {
      value: "=value"
    },
    template: "<div class=\"input-group\"><span class=\"input-group-btn\" ng-click=\"minus()\"><button class=\"btn btn-default\"><span class=\"glyphicon glyphicon-minus\"></span></button></span><input style=\"font-size:x-large\" type=\"text\" class=\"form-control text-center \" ng-model=\"value\" ng-change=\"changed()\"><span class=\"input-group-btn\" ng-click=\"plus()\"><button class=\"btn btn-default\"><span class=\"glyphicon glyphicon-plus\"></span></button></span></div>",
    link: function(scope, element, attributes) {
      var max, min, setValue, step;
      max = void 0;
      min = void 0;
      setValue = void 0;
      step = void 0;
      if (angular.isUndefined(scope.value)) {
        throw "Missing the value attribute on the counter directive.";
      }
      min = (angular.isUndefined(attributes.min) ? null : parseFloat(attributes.min));
      max = (angular.isUndefined(attributes.max) ? null : parseFloat(attributes.max));
      step = (angular.isUndefined(attributes.step) ? 1 : parseFloat(attributes.step));
      element.addClass("counter-container");
      scope.readonly = (angular.isUndefined(attributes.editable) ? true : false);

      /**
      Sets the value as an integer.
       */
      setValue = function(val) {
        scope.value = parseFloat(val);
      };
      setValue(scope.value);

      /**
      Decrement the value and make sure we stay within the limits, if defined.
       */
      scope.minus = function() {
        if (min && (scope.value <= min || scope.value - step <= min) || min === 0 && scope.value < 1) {
          setValue(min);
          return false;
        }
        setValue(scope.value - step);
      };

      /**
      Increment the value and make sure we stay within the limits, if defined.
       */
      scope.plus = function() {
        if (max && (scope.value >= max || scope.value + step >= max)) {
          setValue(max);
          return false;
        }
        setValue(scope.value + step);
      };

      /**
      This is only triggered when the field is manually edited by the user.
      Where we can perform some validation and make sure that they enter the
      correct values from within the restrictions.
       */
      scope.changed = function() {
        if (!scope.value) {
          setValue(0);
        }
        if (/[0-9]/.test(scope.value)) {
          setValue(scope.value);
        } else {
          setValue(scope.min);
        }
        if (min && (scope.value <= min || scope.value - step <= min)) {
          setValue(min);
          return false;
        }
        if (max && (scope.value >= max || scope.value + step >= max)) {
          setValue(max);
          return false;
        }
        setValue(scope.value);
      };
    }
  };
});

})();
