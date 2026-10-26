/* Small, deterministic geometry fixtures for tests; not loaded by the editor. */
exports.terrainFBX = () => Buffer.from(`; FBX 7.4.0 project file
FBXHeaderExtension:  {
	FBXHeaderVersion: 1003
	FBXVersion: 7400
}
GlobalSettings:  {
	Version: 1000
	Properties70:  {
		P: "UpAxis", "int", "Integer", "",1
		P: "UpAxisSign", "int", "Integer", "",1
		P: "FrontAxis", "int", "Integer", "",2
		P: "FrontAxisSign", "int", "Integer", "",1
		P: "CoordAxis", "int", "Integer", "",0
		P: "CoordAxisSign", "int", "Integer", "",1
		P: "UnitScaleFactor", "double", "Number", "",1
	}
}
Objects:  {
	Geometry: 1, "Geometry::SlopedGround", "Mesh" {
		Vertices: *12 {
			a: -100,-27,-100,100,13,-100,100,33,100,-100,-7,100
		}
		PolygonVertexIndex: *6 {
			a: 0,2,-2,0,3,-3
		}
	}
	Model: 2, "Model::SlopedGround", "Mesh" {
		Version: 232
		Properties70:  {
			P: "Lcl Translation", "Lcl Translation", "", "A",0,0,0
			P: "Lcl Rotation", "Lcl Rotation", "", "A",0,0,0
			P: "Lcl Scaling", "Lcl Scaling", "", "A",1,1,1
		}
		Shading: T
		Culling: "CullingOff"
	}
}
Connections:  {
	C: "OO",1,2
	C: "OO",2,0
}
`, "utf8");
