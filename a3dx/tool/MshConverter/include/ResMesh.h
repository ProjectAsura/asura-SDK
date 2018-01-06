﻿//-------------------------------------------------------------------------------------------------
// File : ResMesh.h
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <a3dx_resmesh.h>


namespace a3dx {

//-------------------------------------------------------------------------------------------------
//      メッシュリソースを生成します.
//-------------------------------------------------------------------------------------------------
bool CreateResMeshFromFile(const char* path, a3d::Array<a3d::ResMesh>& meshes);

} // namespace a3dx