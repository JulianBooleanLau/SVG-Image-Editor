'use strict'

// C library API
const ffi = require('ffi-napi');


let sharedLib = ffi.Library('./libsvgparse', {
	'validateSVG'		: [ 'string', [ 'string' ] ],
	'getSVGProperties'	: [ 'string', [ 'string' ] ],
	'getSVGTitleDesc'	: [ 'string', [ 'string' ] ],
	'getSVGRects'		: [ 'string', [ 'string' ] ],
	'getSVGCircs'		: [ 'string', [ 'string' ] ],
	'getSVGPaths'		: [ 'string', [ 'string' ] ],
	'getSVGGroups'		: [ 'string', [ 'string' ] ],
	'changeTitle'		: [ 'string', [ 'string', 'string' ] ],
	'changeDescription'	: [ 'string', [ 'string', 'string' ] ],
	'getRectAttributes'	: [ 'string', [ 'string', 'int' ] ],
	'getCircAttributes'	: [ 'string', [ 'string', 'int' ] ],
	'getPathAttributes'	: [ 'string', [ 'string', 'int' ] ],
	'getGroupAttributes': [ 'string', [ 'string', 'int' ] ],
	
	'editAttributes': [ 'string', [ 'string', 'int', 'string', 'string', 'string' ] ],
	
	
	'createSVG'			: [ 'string', [ 'string' ] ],
	'addRectangle'		: [ 'string', [ 'string', 'string' ] ],
	'addCircle'			: [ 'string', [ 'string', 'string' ] ],
	'scaleCircles'		: [ 'string', [ 'string', 'float' ] ],
	'scaleRectangles'		: [ 'string', [ 'string', 'float' ] ]
});


// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
	if(!req.files) {
		return res.status(400).send('No files were uploaded.');
	}

	let uploadFile = req.files.uploadFile;

	// Use the mv() method to place the file somewhere on your server
	uploadFile.mv('uploads/' + uploadFile.name, function(err) {
		if(err) {
		  return res.status(500).send(err);
		}
		res.redirect('/');
	});
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+ err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

//Validating SVG
app.get('/validateSVG', function(req , res){
	let curStatus = sharedLib.validateSVG("uploads/" + req.query.fileName);
	
	res.send({
		status: curStatus
	});
});

//Getting SVG Properties
app.get('/getSVGProperties', function(req , res){
	let image = sharedLib.getSVGProperties("uploads/" + req.query.fileName);
	
	res.send(image);
});
app.get('/getSVGTitleDesc', function(req , res){
	let image = sharedLib.getSVGTitleDesc("uploads/" + req.query.fileName);
	
	res.send(image);
});
app.get('/getSVGRects', function(req , res){
	let image = sharedLib.getSVGRects("uploads/" + req.query.fileName);
	
	res.send(image);
});
app.get('/getSVGCircs', function(req , res){
	let image = sharedLib.getSVGCircs("uploads/" + req.query.fileName);
	
	res.send(image);
});
app.get('/getSVGPaths', function(req , res){
	let image = sharedLib.getSVGPaths("uploads/" + req.query.fileName);
	
	res.send(image);
});
app.get('/getSVGGroups', function(req , res){
	let image = sharedLib.getSVGGroups("uploads/" + req.query.fileName);
	
	res.send(image);
});

//Changing title/description
app.get('/changeTitle', function(req , res){
	let curStatus = sharedLib.changeTitle("uploads/" + req.query.fileName, req.query.newTitle);
	
	res.send({
		status: curStatus
	});
});
app.get('/changeDescription', function(req , res){
	let curStatus = sharedLib.changeDescription("uploads/" + req.query.fileName, req.query.newDescription);
	
	res.send({
		status: curStatus
	});
});

//Getting Specific Attributes
app.get('/getRectAttributes', function(req , res){
	let image = sharedLib.getRectAttributes("uploads/" + req.query.fileName, req.query.num);
	
	res.send(image);
});
app.get('/getCircAttributes', function(req , res){
	let image = sharedLib.getCircAttributes("uploads/" + req.query.fileName, req.query.num);
	
	res.send(image);
});
app.get('/getPathAttributes', function(req , res){
	let image = sharedLib.getPathAttributes("uploads/" + req.query.fileName, req.query.num);
	
	res.send(image);
});
app.get('/getGroupAttributes', function(req , res){
	let image = sharedLib.getGroupAttributes("uploads/" + req.query.fileName, req.query.num);
	
	res.send(image);
});

//Edit Attributes
app.get('/editAttributes', function(req , res){
	let curStatus = sharedLib.editAttributes("uploads/" + req.query.fileName, req.query.num, req.query.type, req.query.attrName, req.query.attrValue);
	
	res.send({
		status: curStatus
	});
});

//Getting Files already on server
app.get('/getFiles', function(req , res){
	let fs = require('fs');
	let files = fs.readdirSync('uploads/');
	
	res.send(files);
});

//Create SVG
app.get('/createSVG', function(req , res){
	let curStatus = sharedLib.createSVG("uploads/" + req.query.fileName + ".svg");
	
	res.send({
		status: curStatus
	});
});

//Adding shape (rect or circ)
app.get('/addRectangle', function(req , res){
	let curStatus = sharedLib.addRectangle("uploads/" + req.query.fileName, req.query.JSON);
	
	res.send({
		status: curStatus
	});
});
app.get('/addCircle', function(req , res){
	let curStatus = sharedLib.addCircle("uploads/" + req.query.fileName, req.query.JSON);
	
	res.send({
		status: curStatus
	});
});

//Scaling Shapes (Rect or Circ)
app.get('/scaleCircles', function(req , res){
	let curStatus = sharedLib.scaleCircles("uploads/" + req.query.fileName, req.query.scaleFactor);
	
	res.send({
		status: curStatus
	});
});
app.get('/scaleRectangles', function(req , res){
	let curStatus = sharedLib.scaleRectangles("uploads/" + req.query.fileName, req.query.scaleFactor);
	
	res.send({
		status: curStatus
	});
});


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
