// Put all onload AJAX calls here, and event listeners
//After adding shape, creating shape, image doesnt update
//validate files coming from server in update()

$(document).ready(function() {
	console.log("Page successfully loaded");
	
	//Hide adding shape until shape selected
	$('#addCircle').hide();
	$('#addRectangle').hide();
	
	//Variables
    let imageSelect = document.getElementById("imageSelect");
    
    //Load all existing files from server into File Log Panel
	$.ajax({
		url:'/getFiles',
		type: 'GET',
		data: "nothing",
		success: function (data) {
			//Clearing existing file log and image select
			$('#fileLogPanel').find("tr:gt(0)").remove();
			$('#imageSelect').empty();
			$('#imageSelect').append("<option disabled selected value>Click to Select</option>");
			
			//For each file retrieved
			for (let i = 0; i < data.length; ++i) {
				let fileName = data[i];
				var fileSize = 1;
				
				//Getting size of the file
				$.ajax({
					url:'/uploads/' + fileName,
					type: 'GET',
					data: fileName,
					async:false,
					success: function (data) {
						console.log("Success: Files size retrieved");

						var serializer = new XMLSerializer();
						var xmlString = serializer.serializeToString(data);
						fileSize = Math.ceil(xmlString.length / 1024);
						
						//Getting SVG attributes of file
						$.ajax({
							type: 'get',
							dataType: 'json',
							url: '/getSVGProperties',
							async: false,
							data: {
								fileName: fileName
							},
							success: function (data) {
								console.log("Success: File \"" + fileName + "\" retrieved from uploads/");	
														
								//Populating File Log Panel
								let string = "";
								string += "<tr><td><a href=\"" + fileName + "\"><img src=\"" + fileName + "\" width=\"200px\" align=\"middle\"></a></td>";	//Image
								string += "<td><a href=\"" + fileName + "\">" + fileName  + "</a></td>";	//Name (Downloadable)									
								string += "<td>" + fileSize + "</td>";										//Size (KB, rounded)
								string += "<td>" + data.numRect + "</td>";									//Number of Rects
								string += "<td>" + data.numCirc + "</td>";									//Number of Circles
								string += "<td>" + data.numPaths + "</td>";									//Number of Paths
								string += "<td>" + data.numGroups + "</td></tr>";							//Number of Groups
								
								$('#fileLogPanel').append(string);
								
								//Remove the "No Files available message"
								if (noFiles == 1) {
									$('#deleteMe').html("");
									noFiles = 0;
								}
								
								//Updating the image select
								let option = document.createElement("option");
								option.text = fileName;
								imageSelect.add(option);
								
								//Update the SVG image
								let oldImage = $('#SVGViewImage').attr("src");
								$('#SVGViewImage').attr("src", "");
								$('#SVGViewImage').attr("src", oldImage);
							},
							fail: function(error) {
								console.log("Failed: File not Uploaded");
							}
						});		
					},
					fail: function(error) {
						console.log("Failed: File size not retrieved");
						console.log(error);
					}
				});
			}
		},
		fail: function(error) {
			console.log("Failed: No files retrieved from server.");
			console.log(error);
		}
	});

	/* ========== File Log Panel ========== */
	let fileForm = document.getElementById("fileForm");
	let uploadFile = document.getElementById("uploadFile");
	let uploadBtn = document.getElementById("uploadBtn");

	//Uploading selected file
	let noFiles = 1;
	$('#submit').click(function(e){
		e.preventDefault();
		
		//Check if file is selected
		if (uploadFile.files.length == 0) {
			alert("Failed: No file selected");
			return;
		}
		
		var formData = new FormData();
		formData.append("uploadFile", uploadFile.files[0]);
		let fileName = uploadFile.files[0].name;
					
		//Checking if file already exists on the server, if so no uploading
		$.ajax({
			url:'/getFiles',
			type: 'GET',
			data: "nothing",
			success: function (data) {
				console.log("Success: Uploaded file doesn't share a name with another file");
				
				let found = 0;
				for (let i = 0; i < data.length; ++i) {
					let serverFileName = data[i];
					if (fileName == serverFileName) {
						found = 1;
					}
				}
				
				if (found == 1) {
					console.log("Failed: Uploaded file shares the same name as another file");
					alert("Failed: Uploaded file shares the same name as another file");
				} else {
					//Upload selected file
					$.ajax({
						url:'/upload',
						type: 'POST',
						data: formData,
						contentType: false,
						processData: false,

						success: function (data) {
							//Checking if file is a valid SVG
							$.ajax({
								url:'/validateSVG',
								type: 'GET',
								data: {
									fileName: fileName
								},
								success: function (data) {
									if (data.status == "success") {
										console.log("Success: Uploaded file is valid SVG");
										
										//Fill the File Log panel by getting SVG properties
										$.ajax({
											type: 'get',
											dataType: 'json',
											url: '/getSVGProperties',
											data: {
												fileName: fileName
											},
											success: function (data) {
												console.log("Success: File Uploaded");
												
												//Populating File Log Panel
												let string = "";
												string += "<tr><td><a href=\"" + fileName + "\"><img src=\"" + fileName + "\" width=\"200px\" align=\"middle\"></a></td>";	//Image
												string += "<td><a href=\"" + fileName + "\">" + fileName  + "</a></td>";	//Name (Downloadable)									
												string += "<td>" + Math.ceil(uploadFile.files[0].size / 1024) + "</td>";	//Size (KB, rounded)
												string += "<td>" + data.numRect + "</td>";									//Number of Rects
												string += "<td>" + data.numCirc + "</td>";									//Number of Circles
												string += "<td>" + data.numPaths + "</td>";									//Number of Paths
												string += "<td>" + data.numGroups + "</td></tr>";							//Number of Groups
												
												$('#fileLogPanel').append(string);
												
												//Remove the "No Files available message"
												if (noFiles == 1) {
													$('#deleteMe').html("");
													noFiles = 0;
												}
												
												//Updating the image select
												let option = document.createElement("option");
												option.text = fileName;
												imageSelect.add(option);
											},
											fail: function(error) {
												console.log("Failed: File not Uploaded");
											}
										});
									} else {
										console.log("Failed: Invalid SVG, removed from uploads");
										alert("Failed: Invalid SVG, removed from uploads");
									}
									
								},
								fail: function(error) {
									console.log("Failed: File failed to upload");
									alert.log("Failed: File failed to upload");
								}
							});
						},
						fail: function(error) {
							console.log("Failed: File not Uploaded");
						}
					});
				}
			},
			fail: function(error) {
				console.log("Failed: Could not check files on server");
			}
		});
	});
	
	/* ========== SVG View Panel ========== */
	
	//Displays attributes when "Show Attribute" btn is pressed
	function displayAttr(type, fileName, j) {
		return function(event) {
			//console.log("Displaying attr for " + type + " #" + j);
			
			//Clearing table for new values
			$('#attributeTable tr').remove(); //Clearing table
			$('#attributeTable').append("<tr><th>Number</th><th>Name</th><th>Value</th></tr>"); //Adding header back
			
			if (type == "RECT") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/getRectAttributes',
					data: {
						fileName: fileName,
						num: j
					},
					success: function (data) {
						console.log("Success: Retrieved Rectangle attributes ");
						//Getting all attributes
						let i = 1;
						for (let attr of data) {
							let string = "<tr><td>" + i + "</td>";			//Attr number
							string += "<td>" + attr.name + "</td>";			//Attr name
							string += "<td>" + attr.value + "</td></tr>";	//Attr value
							$('#attributeTable').append(string);
							i++;
						}
					},
					fail: function(error) {
						console.log("Failed: Could not retrieve Rectangle attributes ");
						console.log(error);
					}
				});
			} else if (type == "CIRC") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/getCircAttributes',
					data: {
						fileName: fileName,
						num: j
					},
					success: function (data) {
						console.log("Success: Retrieved Circle attributes ");
						//Getting all attributes
						let i = 1;
						for (let attr of data) {
							let string = "<tr><td>" + i + "</td>";			//Attr number
							string += "<td>" + attr.name + "</td>";			//Attr name
							string += "<td>" + attr.value + "</td></tr>";	//Attr value
							$('#attributeTable').append(string);
							i++;
						}
					},
					fail: function(error) {
						console.log("Failed: Could not retrieve Circle attributes ");
						console.log(error);
					}
				});
			} else if (type == "PATH") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/getPathAttributes',
					data: {
						fileName: fileName,
						num: j
					},
					success: function (data) {
						console.log("Success: Retrieved Path attributes ");
						//Getting all attributes
						let i = 1;
						for (let attr of data) {
							let string = "<tr><td>" + i + "</td>";			//Attr number
							string += "<td>" + attr.name + "</td>";			//Attr name
							string += "<td>" + attr.value + "</td></tr>";	//Attr value
							$('#attributeTable').append(string);
							i++;
						}
					},
					fail: function(error) {
						console.log("Failed: Could not retrieve Path attributes ");
						console.log(error);
					}
				});
			} else if (type == "GROUP") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/getGroupAttributes',
					data: {
						fileName: fileName,
						num: j
					},
					success: function (data) {
						console.log("Success: Retrieved Group attributes");
						//Getting all attributes
						let i = 1;
						for (let attr of data) {
							let string = "<tr><td>" + i + "</td>";			//Attr number
							string += "<td>" + attr.name + "</td>";			//Attr name
							string += "<td>" + attr.value + "</td></tr>";	//Attr value
							$('#attributeTable').append(string);
							i++;
						}
					},
					fail: function(error) {
						console.log("Failed: Could not retrieve Group attributes");
						console.log(error);
					}
				});
			} else {
				console.log("Wrong type buddy");
			}
		};
	}
	//Edit attributes when "Edit Attribute" btn is pressed
	function editAttr(type, fileName, j) {
		return function(event) {
			
			//Checking input
			if ($("#editName").val() == "" || $("#editValue").val() == "") {
				console.log("Failed: Invalid attribute to edit");
				alert("Failed: Invalid attribute to edit");
				return;
			}
			
			if (type == "RECT") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/editAttributes',
					data: {
						fileName: fileName,
						num: j,
						type: "RECT",
						attrName: $("#editName").val(),
						attrValue: $("#editValue").val()
					},
					success: function (data) {
						if (data.status == "success") {
							console.log("Success: Edited rectangle attributes");
							
							location.reload(true);
						} else {
							console.log("Failed: Invalid attribute");
							alert("Failed: Invalid attribute");
						}
					},
					fail: function(error) {
						console.log("Failed: Could not edit rectangle attributes ");
						alert("Failed: Could not edit rectangle attributes ");
						console.log(error);
					}
				});
			} else if (type == "CIRC") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/editAttributes',
					data: {
						fileName: fileName,
						num: j,
						type: "CIRC",
						attrName: $("#editName").val(),
						attrValue: $("#editValue").val()
					},
					success: function (data) {
						if (data.status == "success") {
							console.log("Success: Edited circle attributes ");
							
							location.reload(true);
						} else {
							console.log("Failed: Invalid attribute");
							alert("Failed: Invalid attribute");
						}
					},
					fail: function(error) {
						console.log("Failed: Could not edit circle attributes ");
						alert("Failed: Could not edit circle attributes ");
						console.log(error);
					}
				});
			} else if (type == "PATH") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/editAttributes',
					data: {
						fileName: fileName,
						num: j,
						type: "PATH",
						attrName: $("#editName").val(),
						attrValue: $("#editValue").val()
					},
					success: function (data) {
						if (data.status == "success") {
							console.log("Success: Edited path attributes ");
							
							location.reload(true);
						} else {
							console.log("Failed: Invalid attribute");
							alert("Failed: Invalid attribute");
						}
					},
					fail: function(error) {
						console.log("Failed: Could not edit path attributes ");
						alert("Failed: Could not edit path attributes ");
						console.log(error);
					}
				});
			} else if (type == "GROUP") {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/editAttributes',
					data: {
						fileName: fileName,
						num: j,
						type: "GROUP",
						attrName: $("#editName").val(),
						attrValue: $("#editValue").val()
					},
					success: function (data) {
						if (data.status == "success") {
							console.log("Success: Edited group attributes");
							
							location.reload(true);
						} else {
							console.log("Failed: Invalid attribute");
							alert("Failed: Invalid attribute");
						}
					},
					fail: function(error) {
						console.log("Failed: Could not edit group attributes ");
						alert("Failed: Could not edit group attributes ");
						console.log(error);
					}
				});
			}
		};
	}	
						
	//Display current image selected. Allows for showing/editing attributes, title/description, etc
	$('#imageSelect').change(function(e) {
		e.preventDefault();		
		
		let fileName = imageSelect.value;
		console.log("Displaying " + fileName);
		
		//Update image
		$('#SVGViewImage').attr("src", fileName);
		
		//Get title and description
		$.ajax({
			type: 'get',
			dataType: 'json',
			url: '/getSVGTitleDesc',
			data: {
				fileName: fileName
			},
			success: function (data) {
				console.log("Success: Got title and description");
				
				//Updating title and description fields
				$('#title').html(data.title);
				$('#description').html(data.descr);
			},
			fail: function(error) {
				console.log("Failed: Error getting title and description");
			}
		});
		
		//Clearing Table for new values
		$('#componentTable tr').remove(); //Clearing table
		$('#componentTable').append("<tr><th>Component</th><th>Summary</th><th>Other Attributes</th><th>Show Attributes</th><th>Edit Attributes</th></tr>"); //Adding header back
		//Clearing table for new values
		$('#attributeTable tr').remove(); //Clearing table
		$('#attributeTable').append("<tr><th>Number</th><th>Name</th><th>Value</th></tr>"); //Adding header back
		
		//Get SVG Rectangles
		$.ajax({
			type: 'get',
			dataType: 'json',
			url: '/getSVGRects',
			async:false,
			data: {
				fileName: fileName
			},
			success: function (data) {
				console.log("Success: Got SVG Rectanges");
				
				//Updating component table								
				let i = 1;
				for (let rect of data) {
					let string = "<tr>";
					string += "<td>Rectangle " + i + "</td>";	//Component number
					string += "<td>x = " + rect.x + ", y = " + rect.y + ", width = " + rect.w + ", height = " + rect.h + ", units = " + rect.units + "</td>";	//Summary				
					string += "<td>" + rect.numAttr + "</td>";	//Other attributes
					
					//Show/edit attributes buttons
					let showAttr = "showAttrRect" + i;
					string += "<td><button id=\"" + showAttr + "\" type=\"button\">Show Attributes</button></td>"; //Show
					let configAttr =  "configAttrRect" + i;
					string += "<td><button id=\"" + configAttr + "\" type=\"button\">Edit Attributes</button></td></tr>"; //Edit
					$('#componentTable').append(string);
					
					//Show attribute button is clicked
					$("#" + showAttr).bind("click", displayAttr("RECT", fileName, i));
					//Edit attribute button is clicked
					$("#" + configAttr).bind("click", editAttr("RECT", fileName, i));
	
					i++;
				}
			},
			fail: function(error) {
				console.log("Failed: Error getting SVG Rectangles");
			}
		});
		
		//Get SVG Circles
		$.ajax({
			type: 'get',
			dataType: 'json',
			url: '/getSVGCircs',
			async:false,
			data: {
				fileName: fileName
			},
			success: function (data) {
				console.log("Success: Got SVG Circles");
				
				//Updating component table
				let i = 1;
				for (let circle of data) {
					let string = "<tr>";
					string += "<td>Circle " + i + "</td>"; //Component number
					string += "<td>cx = " + circle.cx + ", cy = " + circle.cy + ", r = " + circle.r + ", units = " + circle.units + "</td>";	//Summary				
					string += "<td>" + circle.numAttr + "</td>"; //Other attributes
					
					//Show attributes functionality
					let showAttr = "showAttrCirc" + i;
					string += "<td><button id=\"" + showAttr + "\" type=\"button\">Show Attributes</button></td>"; //Show
					let configAttr =  "configAttrCirc" + i;
					string += "<td><button id=\"" + configAttr + "\" type=\"button\">Edit Attributes</button></td></tr>"; //Edit
					$('#componentTable').append(string);
					
					//When new show attribute button is clicked
					$("#" + showAttr).bind("click", displayAttr("CIRC", fileName, i)); //ajax call to get attributes
					//Edit attribute button is clicked
					$("#" + configAttr).bind("click", editAttr("CIRC", fileName, i));
						
					i++;
				}
			},
			fail: function(error) {
				console.log("Failed: Error getting SVG Circles");
			}
		});
		
		//Get SVG Paths
		$.ajax({
			type: 'get',
			dataType: 'json',
			url: '/getSVGPaths',
			async:false,
			data: {
				fileName: fileName
			},
			success: function (data) {
				console.log("Success: Got SVG Paths");

				//Updating component table
				let i = 1;
				for (let path of data) {
					let string = "<tr>";
					string += "<td>Path " + i + "</td>";		//Component number
					string += "<td>d = " + path.d + "</td>";	//Summary				
					string += "<td>" + path.numAttr + "</td>";	//Other attributes
					
					//Show attributes functionality
					let showAttr = "showAttrPath" + i;
					string += "<td><button id=\"" + showAttr + "\" type=\"button\">Show Attributes</button></td>"; //Show
					let configAttr =  "configAttrPath" + i;
					string += "<td><button id=\"" + configAttr + "\" type=\"button\">Edit Attributes</button></td></tr>"; //Edit
					$('#componentTable').append(string);
					
					//When new show attribute button is clicked
					$("#" + showAttr).bind("click", displayAttr("PATH", fileName, i)); //ajax call to get attributes
					//Edit attribute button is clicked
					$("#" + configAttr).bind("click", editAttr("PATH", fileName, i));

					i++;
				}
			},
			fail: function(error) {
				console.log("Failed: Error getting SVG Paths");
			}
		});
		
		//Get SVG Groups
		$.ajax({
			type: 'get',
			dataType: 'json',
			url: '/getSVGGroups',
			async:false,
			data: {
				fileName: fileName
			},
			success: function (data) {
				console.log("Success: Got SVG Groups");

				//Updating component table
				let i = 1;
				for (let group of data) {
					let string = "<tr>";
					string += "<td>Group " + i + "</td>";	//Component number
					string += "<td>children = " + group.children + "</td>";	//Summary				
					string += "<td>" + group.numAttr + "</td>";	//Other attributes
					
					//Show attributes functionality
					let showAttr = "showAttrGroup" + i;
					string += "<td><button id=\"" + showAttr + "\" type=\"button\">Show Attributes</button></td>"; //Show
					let configAttr =  "configAttrGroup" + i;
					string += "<td><button id=\"" + configAttr + "\" type=\"button\">Edit Attributes</button></td></tr>"; //Edit
					$('#componentTable').append(string);
					
					//When new show attribute button is clicked
					$("#" + showAttr).bind("click", displayAttr("GROUP", fileName, i)); //ajax call to get attributes
					//Edit attribute button is clicked
					$("#" + configAttr).bind("click", editAttr("GROUP", fileName, i));
						
					i++;
				}
			},
			fail: function(error) {
				console.log("Failed: Error getting SVG Group");
			}
		});
	});
	
	//Text area holds the new title/description
    let editTextArea = document.getElementById("editTextArea");
    
	//Updating Title
	let titleBtn = document.getElementById("titleBtn");
	let title = document.getElementById("title");
    titleBtn.addEventListener("click", function(e){
		e.preventDefault();
		
		//No image selected
		if (imageSelect.value == "") {
			console.log("Failed: No image selected to edit title");
			alert("Failed: No image selected to edit title");
			return;
		}
		
		let fileName = imageSelect.value;
		$.ajax({
			type: 'get',            //Request type
			dataType: 'json',       //Data type - we will use JSON for almost everything 
			url: '/changeTitle',   //The server endpoint we are connecting to
			data: {
				fileName: fileName,
				newTitle: editTextArea.value
			},
			success: function (data) {
				if (data.status == "success") {
					console.log("Success: Title updated");
					title.innerHTML = editTextArea.value;
				} else {
					console.log("Failed: Invalid Title");
					alert("Failed: Invalid Title");
				}
			},
			fail: function(error) {
				console.log("Failed: Title cannot be updated");
				alert("Failed: Title cannot be updated");
				console.log(error);
			}
		});
	});

	//Updating Description
	let descBtn = document.getElementById("descBtn");
	let description = document.getElementById("description");
	descBtn.addEventListener("click", function(e){
  		e.preventDefault();
		
		//No image selected
		if (imageSelect.value == "") {
			console.log("Failed: No image selected to edit description");
			alert("Failed: No image selected to edit description");
			return;
		}
		
		let fileName = imageSelect.value;
		$.ajax({
			type: 'get',            //Request type
			dataType: 'json',       //Data type - we will use JSON for almost everything 
			url: '/changeDescription',   //The server endpoint we are connecting to
			data: {
				fileName: fileName,
				newDescription: editTextArea.value
			},
			success: function (data) {
				if (data.status == "success") {
					console.log("Success: Description updated");
					description.innerHTML = editTextArea.value;
				} else {
					console.log("Failed: Invalid Description");
					alert("Failed: Invalid Description");
				}
			},
			fail: function(error) {
				console.log("Failed: Couldn't update description");
				console.log(error);
			}
		});
	});
	
	//Creating SVG
	$('#createSVG').submit(function(e) {
		e.preventDefault();
		
		if ($('#createFileName').val() == "") {
			console.log("Failed: No file Selected");
			alert("Failed: No file Selected");
			return;
		}
		//Call to get all files from server to ensure we dont overwrite an existing file
		$.ajax({
			url:'/getFiles',
			type: 'GET',
			data: "nothing",
			success: function (data) {
				console.log("Success: Files size retrieved")
				
				let sameName = 0;
				//Checking if the created file matches an existing file in name
				for (let i = 0; i < data.length; ++i) {
					if ($('#createFileName').val() + ".svg" == data[i]) {
						sameName = 1;
					}
				}
				
				//Unique SVG being created
				if (sameName == 0) {
					//Call to create SVG
					$.ajax({
						type: 'get',
						dataType: 'json',
						url: '/createSVG',
						data: {
							fileName: $('#createFileName').val()
						},
						success: function (data) {
							if (data.status == "success") {
								console.log("Success: SVG Created");
				
								location.reload(true);
							} else {
								console.log("Failed: Invalid SVG");
								alert("Failed: Invalid SVG");
							}
						},
						fail: function(error) {
							console.log("Failed: Could not create SVG");
							alert("Failed: Could not create SVG");
							console.log(error); 
						}
					});
				} else {
					console.log("Failed: Name already exists");
					alert("Failed: Name already exists");
				}
			},
			fail: function(error) {
				console.log("Failed: Could not retrieve files from server");
				alert("Failed: Could not retrieve files from server");
				console.log(error); 
			}
		});
	});
	
	//Adding shape
	$('#shapeSelect').change(function(e) {
		e.preventDefault();
	
		//Adding rectangle
		if ($('#shapeSelect').val() == "Rectangle" && imageSelect.value != "") {
			//Only show Rectangle Form
			$('#addCircle').hide();
			$('#addRectangle').show();
			
			//User Sumbits form
			$('#addRectangle').submit(function(e) {
				e.preventDefault();
				
				let data = {
					x: parseFloat($("#xVal").val()),
					y: parseFloat($("#yVal").val()),
					w: parseFloat($("#wVal").val()),
					h: parseFloat($("#hVal").val()),					
					units: $("#uValRect").val()
				};
				
				//Get data from the form and send it as JSON
				if ( !(isNaN(data.x) || isNaN(data.y) || isNaN(data.w) || isNaN(data.h)) ) {
					
					//Adding rectangle
					$.ajax({
						type: 'get',
						dataType: 'json',
						url: '/addRectangle',
						data: {
							fileName: imageSelect.value,
							JSON: JSON.stringify(data)
						},
						success: function (data) {
							if (data.status == "success") {
								console.log("Success: Rectangle added");
								
								location.reload(true);
							} else {
								console.log("Failed: Invalid rectangle");
								alert("Failed: Invalid rectangle");
							}
						},
						fail: function(error) {
							console.log("Failed: Could not add rectangle");
							alert("Failed: Could not add rectangle");
							console.log(error); 
						}
					});
				} else {
					console.log("Invalid Rectangle");
					alert("Invalid Rectangle");
				}
			});
		} 
		//Adding circle
		else if ($('#shapeSelect').val() == "Circle" && imageSelect.value != "") {
			//Only show Circle
			$('#addCircle').show();
			$('#addRectangle').hide();
			
			//User Sumbits form
			$('#addCircle').submit(function(e) {
				e.preventDefault();
				
				let data = {
					cx: parseFloat($("#cxVal").val()),
					cy: parseFloat($("#cyVal").val()),
					r: parseFloat($("#rVal").val()),					
					units: $("#uValCirc").val()
				};
				
				//Get data from the form and send it as JSON
				if ( !(isNaN(data.cx) || isNaN(data.cy) || isNaN(data.r)) ) {				
					//Adding circle
					$.ajax({
						type: 'get',
						dataType: 'json',
						url: '/addCircle',
						data: {
							fileName: imageSelect.value,
							JSON: JSON.stringify(data)
						},
						success: function (data) {
							if (data.status == "success") {
								console.log("Success: Circle added");
								
								location.reload(true);
							} else {
								console.log("Failed: Invalid circle");
								alert("Failed: Invalid circle");
							}
						},
						fail: function(error) {
							console.log("Failed: Could not add circle");
							alert("Failed: Could not add circle");
							console.log(error); 
						}
					});
				} else {
					console.log("Invalid Circle");
					alert("Invalid Circle");
				}
			});
		}
	});

	//Scaling Shape
	$('#scaleShape').submit(function(e) {
		e.preventDefault()
		
		//No image selected
		if (imageSelect.value == "") {
			console.log("Failed: No image selected to scale");
			alert("Failed: No image selected to scale");
			return;
		}
		
		//No shape selected
		if ($('#shapeSelect').val() == null) {
			console.log("Failed: No shape selected to scale");
			alert("Failed: No shape selected to scale");
			return;
		}
		
		//Scaling Rect or Circle depending on selected shape
		if ($('#shapeSelect').val() == "Rectangle") {
			let scaleFactor = parseFloat($("#scaleNum").val());
			
			if ( !(isNaN(scaleFactor)) ) {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/scaleRectangles',
					data: {
						fileName: imageSelect.value,
						scaleFactor: scaleFactor
					},
					success: function (data) {
						if (data.status == "success") {
							console.log("Success: Rectangle Scaled");
							
							location.reload(true);
						} else {
							console.log("Failed: Rectangle Not Scaled");
							alert("Failed: Rectangle Not Scaled");
						}
					},
					fail: function(error) {
						console.log("Failed: Rectangle Not Scaled");
						console.log(error); 
					}
				});
			} else {
				console.log("Failed: Invalid scale factor specified");
				alert("Failed: Invalid scale factor specified");
			}
		} else if ($('#shapeSelect').val() == "Circle" && imageSelect.value != "") {
			let scaleFactor = parseFloat($("#scaleNum").val());
			
			if ( !(isNaN(scaleFactor)) ) {
				$.ajax({
					type: 'get',
					dataType: 'json',
					url: '/scaleCircles',
					data: {
						fileName: imageSelect.value,
						scaleFactor: scaleFactor
					},
					success: function (data) {
						if (data.status == "success") {
							console.log("Success: Circle Scaled");
							
							location.reload(true);
						} else {
							console.log("Failed: Circle Not Scaled");
							alert("Failed: Circle Not Scaled");
						}
					},
					fail: function(error) {
						console.log("Failed: Circle Not Scaled");
						console.log(error); 
					}
				});
			} else {
				console.log("Failed: Invalid scale factor specified");
				alert("Failed: Invalid scale factor specified");
			}
		}
	});
	
});
