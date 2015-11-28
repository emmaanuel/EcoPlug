/**
 * Raphael LED gauge
 * @author pzhang
 */

Raphael.fn.ledGauge = function(args) {	
	var paper = this, 
		width = parseInt(window.getComputedStyle(args.div).width),
		height = parseInt(window.getComputedStyle(args.div).height),
		margin_left = (typeof args.margin == 'undefined') ? 
				30 : parseInt(args.margin.left),
		margin_right = (typeof args.margin == 'undefined') ? 
				30 : parseInt(args.margin.right),
		margin_top = (typeof args.margin == 'undefined') ?
				60 : parseInt(args.margin.top),
		margin_bot = (typeof args.margin == 'undefined') ?
				30 : parseInt(args.margin.bottom),
		direction = (typeof args.direction == 'undefined') ?
				0 : args.direction,
		range = args.max - args.min;
		
	var frame = paper.rect(
			margin_left, 
			margin_top, 
			width-(margin_left+margin_right), 
			height-(margin_top+margin_bot))
		.attr({ 	
			stroke: '#cccccc',
			'stroke-width': .1,
			fill: "#000",
			'fill-opacity': .4
		}),
		frame_height = frame.attr("height"),
		frame_width = frame.attr("width"),
		startX = frame.attr("x"),
		startY = frame.attr("y") + frame_height;
	frame.toBack();

	var title = paper.text(
			width/2, 
			20, 
			(typeof args.title == 'undefined')?'':args.title)
		.attr({
			'text-anchor': 'middle',
			'font-size': 16,
			'stroke-width': 3
		});
	var unit = ((typeof args.title == 'undefined') ? '':args.unit),
		bot_number = paper.text(
			width/2,
			startY + 20,
			args.min+unit
		).attr({
			'text-anchor': 'middle',
			'font-size': 16,
			'stroke-width': 3
		});
	if (direction) {
		var mark_1 = paper.path("M"+startX+","+(startY-frame_height)+"v"+(-6)),
		mark_1_value = paper.text(startX, startY-frame_height-12, args.min).attr({
			'text-anchor': 'middle',
			'font-size': 14
		}),
		mark_2 = paper.path("M"+(startX+frame_width/2)+","+
			(startY-frame_height)+"v-6"),
		mark_2_value = paper.text(startX+frame_width/2, 
			startY-frame_height-12, (args.min+args.max)/2)
							.attr({
								'text-anchor': 'middle',
								'font-size': 14
							}),
		mark_3 = paper.path("M"+(startX+frame_width)+","+
			(startY-frame_height)+"v-6"),
		mark_3_value = paper.text(startX+frame_width, 
			startY-frame_height-12, args.max)
							.attr({
								'text-anchor': 'middle',
								'font-size': 14
							});
		//thresholds
		var thresholdValues = args.thresholds.values,
			thresholdColors = args.thresholds.colors,
			thresholdSet = paper.set();
		for (var i = 0; i < thresholdValues.length; i += 1) {
			var percentage = thresholdValues[i]/range;
			var t = paper.rect(
				startX, 
				startY-frame_height,
				frame_width*percentage,
				frame_height
			).attr({
				fill: thresholdColors[i]
			});
			thresholdSet.push(t);
		}
		thresholdSet.forEach(function(shape){
			shape.toBack();
		});

		var bar_number = 60,
			bar_height = frame_height,
			bar_width = frame_width/bar_number,
			initX = startX,
			initY = startY-frame_height,
			isEven = false,
			barSet = paper.set();
		for (var i = 0; i < bar_number; i += 1) {						
			var bar = paper.rect(
				initX, 
				initY, 
				(isEven) ? (bar_width-1) : (bar_width+1), 
				bar_height)
				   .attr({
				   	fill: (isEven) ? '#333' : '',
				   	'stroke-width': .1,
				   	stroke: '#000'
				   });
			barSet.push(bar);
			isEven = !isEven;
			initX = initX + bar_width;
		}
		frame.transform("r180,"+frame.width/2+","+frame.height/2);
		title.attr({
			x: startX,
			y: 20,
			'text-anchor': 'start'
		});		
	} else {
		var mark_1 = paper.path("M"+startX+","+startY+"h"+(-6)),
		mark_1_value = paper.text(startX-5, startY, args.min).attr({
			'text-anchor': 'end',
			'font-size': 14
		}),
		mark_2 = paper.path("M"+startX+","+(startY-frame.attr("height")/2)+"h-6"),
		mark_2_value = paper.text(startX-6, 
			(startY-frame.attr("height")/2), (args.min+args.max)/2)
							.attr({
								'text-anchor': 'end',
								'font-size': 14
							}),
		mark_3 = paper.path("M"+startX+","+(startY-frame.attr("height"))+"h-6"),
		mark_3_value = paper.text(startX-6, 
			(startY-frame.attr("height")), args.max)
							.attr({
								'text-anchor': 'end',
								'font-size': 14
							});
		//thresholds
		var thresholdValues = args.thresholds.values,
			thresholdColors = args.thresholds.colors,
			thresholdSet = paper.set();
		for (var i = 0; i < thresholdValues.length; i += 1) {
			var percentage = thresholdValues[i]/100;
			var t = paper.rect(
				startX, 
				startY-frame_height*percentage,
				frame_width,
				frame_height*percentage
			).attr({
				fill: thresholdColors[i]
			});
			thresholdSet.push(t);
		}
		thresholdSet.forEach(function(shape){
			shape.toBack();
		});

		var bar_number = 60,
			bar_height = frame_height/bar_number,
			bar_width = frame_width,
			initX = startX,
			initY = startY,
			isEven = false,
			barSet = paper.set();
		for (var i = 0; i < bar_number; i += 1) {
			initY = initY - bar_height;
			var bar = paper.rect(
				initX, 
				initY, 
				bar_width, 
				(isEven) ? (bar_height-1) : (bar_height+1))
						   .attr({
						   	fill: (isEven) ? '#333' : '',
						   	'stroke-width': .1,
						   	stroke: '#000'
						   });
			barSet.push(bar);
			isEven = !isEven;
		}
	}
	
	this.setTo = function(value) {
		var bright_percentage = value/(args.max-args.min),
			dark_percentage = 1 - bright_percentage,
			newHeight = dark_percentage*frame_height,
			newWidth = dark_percentage*frame_width;
		if (direction) {
			frame.animate({
				width: newWidth
			}, 600);
		} else {
			frame.animate({
				height: newHeight
			}, 600);		
		}		
		bot_number.attr({
			text: value + ((typeof args.title == 'undefined') ? '':args.unit)
		});
	};
	return this;
};