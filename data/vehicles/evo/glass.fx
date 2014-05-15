

texture diffuseMap {
	type = TEX_2D;
	src = "glass.png";
}


technique TestTechnique {

	pass {
		depthtest = true;

		cullface = CW;
		
		blendmode = ALPHA;

		texunit0 = diffuseMap;
	}


}



