

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
        $scope.score = 0;
        $scope.led = Raphael('gauge', 100, 500).ledGauge({
            div: document.getElementById('gauge'),
            min: 0,
            max: 500,
            title: 'Juliette',
            unit: '',
            direction: 0,
            margin: {
                left: '40px',
                right: '30px',
                top: '40px',
                bottom: '40px'
            },
            thresholds: {
                values: [20, 40, 100],
                colors: ['#ff0000', '#f0f000', '#00ff00']
            }
        });

        getScore = function() {
            $http({method : 'GET',url : '/api/bad'})
            .success(function(data, status) {
                data.players.forEach(function(element, index, array){
                    $scope.led.setTo(element.speed);
                    $scope.score = element.score;
                });
            })
            .error(function(data, status) {
                console.log("Error");
            });
        };

        $scope.resetScore = function() {
            $http({method : 'DELETE',url : '/api/bad'}).error(function(data, status) {
                console.log("Error");
            });
        };

        secondIntervalFunction = function(){
            $timeout(function() {
                getScore();
                secondIntervalFunction();
            }, 800)
        };    
        secondIntervalFunction();
    }]);
})();
