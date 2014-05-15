

texture diffuseMap {
    type = TEX_2D;
    src = "seat_complete.jpg";
}


technique TestTechnique {

    pass {
        depthtest = true;

        cullface = CW;

        texunit0 = diffuseMap;
    }


}



