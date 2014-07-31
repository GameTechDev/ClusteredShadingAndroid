#include "ClusteredShadingSample.h"
#include "CPUTRenderStateBlockOGL.h"
#include "CPUTGuiControllerOGL.h"
#include "CPUTTextureOGL.h"
#include "CPUTFont.h"
#include "ColorUtil.h"
#include "FragmentFactory.h"

const UINT SHADOW_WIDTH_HEIGHT = 512;

#define ASSET_LOCATION   _L("building/")
#define ASSET_SET_FILE   _L("building_05")

#ifdef CPUT_OS_WINDOWS
void APIENTRY openglCallbackFunction(GLenum source,
                                           GLenum type,
                                           GLuint id,
                                           GLenum severity,
                                           GLsizei length,
                                           const GLchar* message,
                                           void* userParam){
 
    std::cout << "---------------------opengl-callback-start------------" << std::endl;
    std::cout << "message: "<< message << std::endl;
    std::cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "OTHER";
        break;
    }
    std::cout << std::endl;
 
    std::cout << "id: "<< id << std::endl;
    std::cout << "severity: " << severity << std::endl;
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "HIGH";
        break;
    }
    std::cout << std::endl;
    std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}
#endif


// Handle OnCreation events
//-----------------------------------------------------------------------------
void ClusteredShadingSample::Create()
{    
#ifdef CPUT_OS_WINDOWS
    glDebugMessageCallback(openglCallbackFunction, NULL);
#endif

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    mUIConstants.lightingOnly = 0;
    mUIConstants.faceNormals = 0;
    mUIConstants.visualizeLightCount = 0;
    mUIConstants.visualizePerSampleShading =0 ;
    
    mUIConstants.lightCullTechnique = CULL_CLUSTERED;
    mUIConstants.clusteredGridScale = 16;
    mUIConstants.Dummy0 = 0;
    mUIConstants.Dummy1 = 0;

    {
        cString name = _L("$ui_constants");
        mpUIConstantsBuffer = new CPUTBufferOGL(name, GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, 32 + sizeof(mUIConstants), nullptr );
        pAssetLibrary->AddConstantBuffer(_L(""), name, _L(""), mpUIConstantsBuffer);
    }

    cString mediaDirectory;

#ifdef CPUT_OS_WINDOWS
    cString executableDirectory;
    CPUTFileSystem::GetExecutableDirectory(&executableDirectory);
    CPUTFileSystem::ResolveAbsolutePathAndFilename(executableDirectory + _L("../../../Media/"), &mediaDirectory);
#endif

#ifdef CPUT_OS_ANDROID
    android_app *pState = CPUTWindowAndroid::GetAppState();
    cString guiDirectory;
    cString systemDirectory;
#   ifdef CPUT_USE_ANDROID_ASSET_MANAGER
        CPUTFileSystem::GetExecutableDirectory(&mediaDirectory);
#   else
        ANativeActivity* nativeActivity = pState->activity;                              
        const char* externalDataPath = nativeActivity->externalDataPath;
        mediaDirectory.assign(externalDataPath);
	    mediaDirectory.append("/Media/");
#   endif
#endif

    pAssetLibrary->SetMediaDirectoryName(mediaDirectory);
    pAssetLibrary->SetSystemDirectoryName(mediaDirectory + _L("System/"));

    CPUTGuiControllerOGL *pGUI = (CPUTGuiControllerOGL*)CPUTGetGuiController();
    pAssetLibrary->SetAssetDirectoryName(mediaDirectory + _L("gui_assets/"));
    pGUI->Initialize(mediaDirectory);
    pGUI->SetCallback(this);
    pGUI->SetWindow(mpWindow);

    CPUTFont *pFont = CPUTFont::CreateFont(mediaDirectory + _L("System/Font/"), _L("arial_64.fnt"));
    
#ifdef CPUT_OS_WINDOWS
    pAssetLibrary->AddFont(_L("DefaultFont"), _L(""), _L(""), pFont);
#endif
#ifdef CPUT_OS_ANDROID
    pAssetLibrary->GetFontByName(_L("arial_64.fnt"));
#endif
    pGUI->SetFont(pFont);

    //
    // Create some controls
    //   
	pGUI->CreateDropdown(_L("Technique: forward"), ID_SHADING_TECH_DROPDOWN, ID_MAIN_PANEL, &mpShadingTechDropdown);
    mpShadingTechDropdown->AddSelectionItem(_L("Technique: deferred") );
    mpShadingTechDropdown->AddSelectionItem(_L("Technique: quad") );
    mpShadingTechDropdown->AddSelectionItem(_L("Technique: clustered") );
    mpShadingTechDropdown->AddSelectionItem(_L("Technique: CS tile") );
    mpShadingTechDropdown->SetSelectedItem(mUIConstants.lightCullTechnique);

    int iNumLightsPow = 7;
    int iNumLights = 1<<iNumLightsPow;
    CPUTSlider *pNumLightSlider = nullptr;
#if defined(CPUT_OS_ANDROID)
    std::stringstream ssNumLights;
#elif defined(CPUT_OS_WINDOWS)
    std::wstringstream ssNumLights;
#endif
    ssNumLights << _L("Num lights: ") << iNumLights;
    pGUI->CreateSlider(ssNumLights.str(), ID_NUM_LIGHTS_SLIDER, ID_MAIN_PANEL, &pNumLightSlider, 2.0f);
    pNumLightSlider->SetScale(0, 10, 11);
    pNumLightSlider->SetValue((float)iNumLightsPow);
    
    pGUI->CreateCheckbox(_L("Animate Lights"), ID_ANIMATE_LIGHTS_CHECKBOX, ID_MAIN_PANEL, &mpAnimateLightsCheckbox, 2.0f);
    mpAnimateLightsCheckbox->SetCheckboxState(CPUT_CHECKBOX_CHECKED);
    
    //CPUTCheckbox *pLightingOnlyCheckbox;
    //pGUI->CreateCheckbox(_L("Lighting only"), ID_LIGHTING_ONLY_CHECKBOX, ID_MAIN_PANEL, &pLightingOnlyCheckbox);
    //pLightingOnlyCheckbox->SetCheckboxState(mUIConstants.lightingOnly ? CPUT_CHECKBOX_CHECKED : CPUT_CHECKBOX_UNCHECKED);

    CPUTCheckbox *pVisualizeLightCountCheckbox;
    pGUI->CreateCheckbox(_L("Visualize light count"), ID_VISUALIZE_LIGHT_COUNT_CHECKBOX, ID_MAIN_PANEL, &pVisualizeLightCountCheckbox, 2.0f);
    pVisualizeLightCountCheckbox->SetCheckboxState(mUIConstants.visualizeLightCount ? CPUT_CHECKBOX_CHECKED : CPUT_CHECKBOX_UNCHECKED);

    InitializeLightParameters();
    SetActiveLights(iNumLights);
	// initialize Cluster Size parameters
    mLightGridBuilder.reset(LightGridDimensions(32, 16, 128));
    
    int width, height;
    mpWindow->GetClientDimensions(&width, &height);

    CPUTTextureOGL*  pDepthTexture = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$shadow_depth"), GL_DEPTH_COMPONENT, SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    mpShadowRenderTarget = new CPUTFramebufferOGL(NULL, pDepthTexture);
    SAFE_RELEASE(pDepthTexture);

    mpLightGridTexture = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$light_grid"), GL_RGBA32UI, LIGHT_GRID_TEXTURE_WIDTH, LIGHT_GRID_TEXTURE_HEIGHT, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
    mLightGridTextureData.resize(LIGHT_GRID_TEXTURE_WIDTH * LIGHT_GRID_TEXTURE_HEIGHT * 4);

    // Call ResizeWindow() because it creates some resources that our blur material needs (e.g., the back buffer)	
    ResizeWindow(width, height);

	mpScene = new CPUTScene();
    
#ifdef CPUT_OS_ANDROID
    DEBUG_PRINT(_L("Load Scene"));
#endif
    pAssetLibrary->SetAssetDirectoryName( mediaDirectory + ASSET_LOCATION );
    CPUTAssetSet *pAssetSet = pAssetLibrary->GetAssetSet( ASSET_SET_FILE );
    mpScene->AddAssetSet(pAssetSet);
    pAssetSet->Release();
#ifdef CPUT_OS_ANDROID
    LOGI("Loaded the scene");
#endif

    const cString MaterialNames[] = 
    {
        _L("concreteroof.mtl"),
        _L("concretewallsturtle.mtl"),
        _L("levelgridframes.mtl"),
        _L("levelmetal.mtl"),
        _L("metalgrunge.mtl"),
        _L("roofpipes.mtl"),
        _L("roofvents1.mtl"),
        _L("tilefloorturtle.mtl"),
        _L("tilewallsturtle.mtl"),
        _L("ventpipes1.mtl"),
        _L("vents.mtl"),
        _L("windows.mtl")
    };
    for( int Mtrl = 0; Mtrl < sizeof(MaterialNames)/sizeof(MaterialNames[0]); ++Mtrl)
    {
        mpMaterials.push_back( pAssetLibrary->GetMaterialByName( MaterialNames[Mtrl] ) );
    }

    // Get the camera. Get the first camera encountered in the scene or
    // if there are none, create a new one.
    unsigned int numAssets = mpScene->GetNumAssetSets();
    for (unsigned int i = 0; i < numAssets; ++i) {
        CPUTAssetSet *pAssetSet = mpScene->GetAssetSet(i);
        if (pAssetSet->GetCameraCount() > 0) {
            mpCamera = pAssetSet->GetFirstCamera();
            break;
        }
    }
    
    if (mpCamera == NULL) {
        mpCamera = new CPUTCamera();
        pAssetLibrary->AddCamera( _L(""), _L("Default Camera"),_L(""),  mpCamera );

        mpCamera->SetPosition( 0.0f, 10.0f, 0.0f );
        // Set the projection matrix for all of the cameras to match our window.
        // Note: this should really be a viewport matrix.  Otherwise, all cameras will have the same FOV and aspect ratio, etc instead of just viewport dimensions.
        mpCamera->SetAspectRatio(((float)width)/((float)height));
    }
    mpCamera->SetFov(DegToRad(90.0));
    mpCamera->SetFarPlaneDistance(100.0f);
    mpCamera->SetNearPlaneDistance(0.1f);
    mpCamera->Update();

    // Position and orient the shadow camera so that it sees the whole scene.
    // Set the near and far planes so that the frustum contains the whole scene.
    // Note that if we are allowed to move the shadow camera or the scene, this may no longer be true.
    // Consider updating the shadow camera when it (or the scene) moves.
    // Treat bounding box half as radius for an approximate bounding sphere.
    // The tightest-fitting sphere might be smaller, but this is close enough for placing our shadow camera.
    float3 sceneCenterPoint, halfVector;
    mpScene->GetBoundingBox(&sceneCenterPoint, &halfVector);
    float  length = halfVector.length();
    mpShadowCamera = new CPUTCamera( CPUT_ORTHOGRAPHIC );
    mpShadowCamera->SetAspectRatio(1.0f);
    mpShadowCamera->SetNearPlaneDistance(1.0f);
    mpShadowCamera->SetFarPlaneDistance(2.0f*length + 1.0f);
    mpShadowCamera->SetPosition( sceneCenterPoint - float3(0, -1, 1) * length );
    mpShadowCamera->LookAt( sceneCenterPoint );
    mpShadowCamera->SetWidth( length*2);
    mpShadowCamera->SetHeight(length*2);
    mpShadowCamera->Update();

    pAssetLibrary->AddCamera( _L("ShadowCamera"), _L(""), _L(""), mpShadowCamera );
    mpCameraController = new CPUTCameraControllerFPS();
    mpCameraController->SetCamera(mpCamera);
    mpCameraController->SetLookSpeed(0.004f);
    mpCameraController->SetMoveSpeed(20.0f);

    mpDebugSprite = CPUTSprite::CreateSprite( -1.0f, -1.0f, 0.5f, 0.5f, _L("%spriteOGL") );

    
    cString ShaderDirectory = mediaDirectory + _L("building/Shader/");
    std::vector< cString > Includes;
    Includes.push_back( _L("PerFrameConstants.h") );
    Includes.push_back( _L("ShaderDefines.h") );
    Includes.push_back( _L("Rendering.h") );
    Includes.push_back( _L("GBuffer.h") );
    Includes.push_back( _L("FramebufferFlat.h") );
    m_DeferredNoCullProgram.CreateProgram(ShaderDirectory, Includes, _L("FullScreenTriangleVS.glsl"), _L("BasicLoopFS.glsl") );

    Includes.push_back( _L("GPUQuad.h") );
    m_GPUQuadProgram.CreateProgram(ShaderDirectory, Includes, _L("GPUQuadVS.glsl"), _L("GPUQuadFS.glsl") );
	m_TiledDeferredCSProgram.CreateProgram(ShaderDirectory, Includes, _L(""), _L(""), _L("TiledDeferredCS.glsl"));
	m_CopyTextureProgram.CreateProgram(ShaderDirectory, std::vector< cString >(), _L("FullScreenTriangleVS.glsl"), _L("CopyTextureFS.glsl"));
}

void ClusteredShadingSample::SetActiveLights(unsigned int activeLights)
{
    mActiveLights = activeLights;
    
    if( mpPointLightsInfoTexture )
        mpPointLightsInfoTexture->InitGLTexture(GL_RGBA32F, mActiveLights, 2, GL_RGBA, GL_FLOAT, NULL);
    else
        mpPointLightsInfoTexture = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$lights_buffer"), GL_RGBA32F, mActiveLights, 2, GL_RGBA, GL_FLOAT, NULL);
    
    // Make sure all the active lights are set up
    Move(0.0f);
}


class CRndHelper
{
public:
    CRndHelper(float fMin, float fMax) : 
        m_fMin(fMin), 
        m_fMax(fMax)
    {}

    float operator() ()
    {
        return ((float)rand() / (float)RAND_MAX) * (m_fMax - m_fMin) + m_fMin;
    }

private:
    float m_fMin, m_fMax;
};

void ClusteredShadingSample::InitializeLightParameters()
{
    mPointLightParameters.resize(MAX_LIGHTS);
    mLightsInfoTextureData.resize(MAX_LIGHTS * 8 );

    mLightInitialTransform.resize(MAX_LIGHTS);
    mPointLightPositionWorld.resize(MAX_LIGHTS);

    // Use a constant seed for consistency
    srand(1337);

    CRndHelper radiusNormDist(0.0f, 1.0f);
    const float maxRadius = 50.0f;
    CRndHelper angleDist(0.0f, 2.0f * PI); 
    CRndHelper heightDist(2.0f, 18.0f);
    CRndHelper animationSpeedDist(2.0f, 20.0f);
    CRndHelper animationDirection(0, 1);
    CRndHelper hueDist(0.0f, 1.0f);
    CRndHelper intensityDist(0.1f, 0.5f);
    CRndHelper attenuationDist(4.0f, 10.0f);
    const float attenuationStartFactor = 0.8f;
    
    for (unsigned int i = 0; i < MAX_LIGHTS; ++i) {
        PointLight& params = mPointLightParameters[i];
        PointLightInitTransform& init = mLightInitialTransform[i];

        init.radius = std::sqrt(radiusNormDist()) * maxRadius;
        init.angle = angleDist();
        init.height = heightDist();
        // Normalize by arc length
        init.animationSpeed = (animationDirection() > 0.5f ? +1.f : -1.f) * animationSpeedDist() / init.radius;
        
        // HSL->RGB, vary light hue
        params.color = intensityDist() * HueToRGB(hueDist());
        params.attenuationEnd = attenuationDist();
        params.attenuationBegin = attenuationStartFactor * params.attenuationEnd;
    }
}

void ClusteredShadingSample::ClusterCullingRasterizeLights(const float4x4& mCameraProj)
{
    auto n = mUIConstants.clusteredGridScale;
    mLightGridBuilder.reset(LightGridDimensions(2*n, n, 8*n));

	{
        //int64_t raster_clk = -get_tsc();
        {
            mLightGridBuilder.clearAllFragments();
            RasterizeLights(&mLightGridBuilder, mCameraProj, &mPointLightParameters[0], mActiveLights);
        }
        //raster_clk += get_tsc();

        //int64_t upload_clk = -get_tsc();
        {
            mLightGridBuilder.buildAndUpload(&mLightGridTextureData[0], LIGHT_GRID_TEXTURE_WIDTH * LIGHT_GRID_TEXTURE_HEIGHT * sizeof(unsigned int) * 4);
            mpLightGridTexture->UpdateData(&mLightGridTextureData[0], GL_RGBA_INTEGER, GL_UNSIGNED_INT);
        }
        //upload_clk += get_tsc();

        //dprintf("rasterization: %.2f clk/entry \n", 1.0*raster_clk / entry_count);
        //dprintf("grid build: %.2f clk/entry \n", 1.0*list_clk / entry_count);
        //dprintf("grid size: %.2f MB \n", 1.0*mLightGridBuilder.allocatedBytes / 1024/1024);
        //dprintf("grid mapd: %.2f clk/entry \n", 1.0*upload_clk / entry_count);
    }
}

void ClusteredShadingSample::UpdateLights(const float4x4& cameraView)
{
    assert( mActiveLights < MAX_LIGHTS );
    // Transform light world positions into view space and store in our parameters array
    for(unsigned int iLight = 0; iLight < mActiveLights; ++iLight)
    {
        float4 LighWorldPos(mPointLightPositionWorld[iLight], 1.f);
        float4 LightViewPos = LighWorldPos * cameraView;
        LightViewPos.x /= LightViewPos.w;
        LightViewPos.y /= LightViewPos.w;
        LightViewPos.z /= LightViewPos.w;
        auto &CurrPointLightParams = mPointLightParameters[iLight];
        CurrPointLightParams.positionView = LightViewPos;
        mLightsInfoTextureData[iLight*4 + 0] = LightViewPos.x;
        mLightsInfoTextureData[iLight*4 + 1] = LightViewPos.y;
        mLightsInfoTextureData[iLight*4 + 2] = LightViewPos.z;
        mLightsInfoTextureData[iLight*4 + 3] = CurrPointLightParams.attenuationBegin;

        mLightsInfoTextureData[iLight*4 + mActiveLights * 4 + 0] = CurrPointLightParams.color.x;
        mLightsInfoTextureData[iLight*4 + mActiveLights * 4 + 1] = CurrPointLightParams.color.y;
        mLightsInfoTextureData[iLight*4 + mActiveLights * 4 + 2] = CurrPointLightParams.color.z;
        mLightsInfoTextureData[iLight*4 + mActiveLights * 4 + 3] = CurrPointLightParams.attenuationEnd;
    }
   
    // Copy light list into shader buffer
    {
        mpPointLightsInfoTexture->UpdateData(&mLightsInfoTextureData[0], GL_RGBA, GL_FLOAT);
    }
}


void ClusteredShadingSample::Move(float elapsedTime)
{
    mTotalTime += elapsedTime;
    
    // Update positions of active lights
    for (unsigned int i = 0; i < mActiveLights; ++i) {
        const PointLightInitTransform& initTransform = mLightInitialTransform[i];
        float angle = initTransform.angle + mTotalTime * initTransform.animationSpeed;
        mPointLightPositionWorld[i] = float3(
            initTransform.radius * std::cos(angle),
            initTransform.height,
            initTransform.radius * std::sin(angle));
    }
}

void ClusteredShadingSample::RenderScene(CPUTRenderParameters &renderParams, int MaterialIndex)
{
    for(auto It = mpMaterials.begin(); It!=mpMaterials.end(); ++It)
        (*It)->SetCurrentEffect(MaterialIndex);
    mpScene->Render( renderParams );	
}

void ClusteredShadingSample::RenderForward(CPUTRenderParameters &renderParams)
{
    RenderScene(renderParams, MATERIAL_FORWARD);
}

void ClusteredShadingSample::RenderForwardClustered(CPUTRenderParameters &renderParams)
{
    ClusterCullingRasterizeLights(*mpCamera->GetProjectionMatrix() );
    RenderScene(renderParams, MATERIAL_CLUSTERED);
}

void ClusteredShadingSample::RenderGBuffer(CPUTRenderParameters &renderParams)
{
    GL_CHECK( glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferFBO) );

    int x, y, w, h;
    mpWindow->GetClientDimensions(&x, &y, &w, &h);
    GL_CHECK(glViewport(0, 0, w, h ));
    GL_CHECK(glClearColor ( 0.0f, 0.02f, 0.05f, 1e+5 ));
    GL_CHECK(glClearDepthf(0.0f));
    GL_CHECK(glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));

    RenderScene(renderParams, MATERIAL_GBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ClusteredShadingSample::SetGLProgramUniforms(GLenum GLProgram, bool BindLightsBuffer/* = true*/)
{
	auto DiffuseSamplerLocation = glGetUniformLocation(GLProgram, "gGBufferDiffuse");
	int DiffuseSamplerBindPoint = 0;
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + DiffuseSamplerBindPoint));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, mpGBufferDiffuseColor->GetTexture()));
	glUniform1i(DiffuseSamplerLocation, DiffuseSamplerBindPoint);

	auto NormalSamplerLocation = glGetUniformLocation(GLProgram, "gGBufferNormal");
	int NormalSamplerBindPoint = 1;
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + NormalSamplerBindPoint));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, mpGBufferNormal->GetTexture()));
	glUniform1i(NormalSamplerLocation, NormalSamplerBindPoint);

	auto LightMapSamplerLocation = glGetUniformLocation(GLProgram, "gGBufferLightMap");
	int LightMapSamplerBindPoint = 2;
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + LightMapSamplerBindPoint));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, mpGBufferLightMap->GetTexture()));
	glUniform1i(LightMapSamplerLocation, LightMapSamplerBindPoint);

	//auto DepthBufferSamplerLocation = glGetUniformLocation(m_DeferredNoCullProgram.GetProgram(), "gDepthBuffer");
	//int DepthBufferSamplerBindPoint = 3;
	//GL_CHECK(glActiveTexture(GL_TEXTURE0 + DepthBufferSamplerBindPoint));
	//GL_CHECK(glBindTexture(GL_TEXTURE_2D, mpGBufferDepth->GetTexture()));
	//glUniform1i(DepthBufferSamplerLocation, DepthBufferSamplerBindPoint); 

	auto LightsBufferSamplerLocation = glGetUniformLocation(GLProgram, "gLightsBuffer");
	int LightsBufferSamplerBindPoint = 4;
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + LightsBufferSamplerBindPoint));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, BindLightsBuffer ? mpPointLightsInfoTexture->GetTexture() : 0));
	glUniform1i(LightsBufferSamplerLocation, LightsBufferSamplerBindPoint);

	auto PerFrameValuesCBLocation = glGetUniformBlockIndex(GLProgram, "cbPerFrameValues");
	GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, PerFrameValuesCBLocation, mpPerFrameConstantBuffer->GetBufferID()));
	GL_CHECK(glUniformBlockBinding(GLProgram, PerFrameValuesCBLocation, PerFrameValuesCBLocation));

	auto UIConstantsCBLocation = glGetUniformBlockIndex(GLProgram, "cbUIConstants");
	GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, UIConstantsCBLocation, mpUIConstantsBuffer->GetBufferID()));
	GL_CHECK(glUniformBlockBinding(GLProgram, UIConstantsCBLocation, UIConstantsCBLocation));
}

void ClusteredShadingSample::ComputeLighting(CPUTRenderParameters &renderParams)
{
	GL_CHECK(glBindVertexArray(m_DummyVAO));

	if (mUIConstants.lightCullTechnique == CULL_QUAD || 
		mUIConstants.lightCullTechnique == CULL_DEFERRED_NONE)
    {
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glUseProgram(m_DeferredNoCullProgram.GetProgram()));
		// When quad mode is selected, we stil need to apply the light maps.
		// To do this, we use DeferredNoCull, but set gLightsBuffer uniform to 0.
		// This gives us the desired effect
		SetGLProgramUniforms(m_DeferredNoCullProgram.GetProgram(), mUIConstants.lightCullTechnique == CULL_DEFERRED_NONE);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		GL_CHECK(glEnable(GL_DEPTH_TEST));
    }
	
	if( mUIConstants.lightCullTechnique == CULL_QUAD )
	{
		GL_CHECK(glUseProgram(m_GPUQuadProgram.GetProgram()));
		SetGLProgramUniforms(m_GPUQuadProgram.GetProgram());

		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
		GL_CHECK(glDepthMask(0));

		glDrawArrays(GL_TRIANGLES, 0, mActiveLights * 6);

		GL_CHECK(glDepthMask(1));
		GL_CHECK(glDisable(GL_BLEND));
	}

	if (mUIConstants.lightCullTechnique == CULL_COMPUTE_SHADER_TILE )
	{
		GL_CHECK(glUseProgram(m_TiledDeferredCSProgram.GetProgram()));
		SetGLProgramUniforms(m_TiledDeferredCSProgram.GetProgram());

		GL_CHECK(glBindImageTexture(0, mpShadedBackBuffer->GetTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F));
		unsigned int dispatchWidth = (mBackBufferWidth + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
		unsigned int dispatchHeight = (mBackBufferHeight + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
		GL_CHECK(glDispatchCompute(dispatchWidth, dispatchHeight, 1));

		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glUseProgram(m_CopyTextureProgram.GetProgram()));
		{
			auto SrcSamplerLocation = glGetUniformLocation(m_CopyTextureProgram.GetProgram(), "gSrcTex");
			int SrcSamplerBindPoint = 0;
			GL_CHECK(glActiveTexture(GL_TEXTURE0 + SrcSamplerBindPoint));
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, mpShadedBackBuffer->GetTexture() ));
			glUniform1i(SrcSamplerLocation, SrcSamplerBindPoint);
		}
		glDrawArrays(GL_TRIANGLES, 0, 3);
		GL_CHECK(glEnable(GL_DEPTH_TEST));
    }

	GL_CHECK(glBindVertexArray(0));
	GL_CHECK(glUseProgram(0));
}

//-----------------------------------------------------------------------------
void ClusteredShadingSample::Render(double deltaSeconds)
{
    mpUIConstantsBuffer->SetSubData(32, sizeof(mUIConstants), &mUIConstants);

    UpdateLights( *mpCamera->GetViewMatrix() );

    CPUTRenderParameters renderParams;
    renderParams.mpPerFrameConstants = mpPerFrameConstantBuffer;
    renderParams.mpPerModelConstants = mpPerModelConstantBuffer;
    renderParams.mpCamera = mpCamera;
    UpdatePerFrameConstantBuffer(renderParams, deltaSeconds);
    int x, y, w, h;
    mpWindow->GetClientDimensions(&x, &y, &w, &h);
    renderParams.mWidth = w;
    renderParams.mHeight = h;
    GL_CHECK(glViewport(0, 0, w, h ));
    GL_CHECK(glClearColor ( 0.0f, 0.02f, 0.05f, 1 ));
    GL_CHECK(glClearDepthf(0.0f));
    GL_CHECK(glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));

    if( mUIConstants.lightCullTechnique == CULL_FORWARD_NONE )
    {
        RenderForward(renderParams);
    }
    else if( mUIConstants.lightCullTechnique == CULL_CLUSTERED )
    {
        RenderForwardClustered(renderParams);
    }
    else
    {
        RenderGBuffer(renderParams);
        ComputeLighting(renderParams);
    }
   

    CPUTDrawGUI();
}


// Handle resize events
//-----------------------------------------------------------------------------
void ClusteredShadingSample::ResizeWindow(UINT width, UINT height)
{
	mBackBufferWidth  = width;
	mBackBufferHeight = height;

    SAFE_RELEASE( mpGBufferDiffuseColor );
    SAFE_RELEASE( mpGBufferNormal );
    SAFE_RELEASE( mpGBufferLightMap );
    SAFE_RELEASE( mpGBufferDepth );
	SAFE_RELEASE( mpShadedBackBuffer );

    mpGBufferDiffuseColor = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$GBufferDiffuse"), GL_RGBA16F, width, height, GL_RGBA, GL_HALF_FLOAT, NULL);
    mpGBufferNormal = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$GBufferNormal"), GL_RG16F, width, height, GL_RG, GL_HALF_FLOAT, NULL);
    mpGBufferLightMap = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$GBufferLightMap"), GL_RGBA16F, width, height, GL_RGBA, GL_HALF_FLOAT, NULL);
    mpGBufferDepth = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$GBufferDepth"), GL_DEPTH_COMPONENT32F, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	mpShadedBackBuffer = (CPUTTextureOGL*)CPUTTextureOGL::CreateTexture(_L("$GLitBackBuffer"), GL_RGBA16F, width, height, GL_RGBA, GL_HALF_FLOAT, NULL, true);


    if( !m_GBufferFBO )
        GL_CHECK( glGenFramebuffers(1, &m_GBufferFBO) );
    if( !m_DummyVAO )
        GL_CHECK( glGenVertexArrays(1, &m_DummyVAO) );

    GL_CHECK( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFBO) );
    GL_CHECK( glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mpGBufferDiffuseColor->GetTexture(), 0) );
    GL_CHECK( glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mpGBufferNormal->GetTexture(), 0) );
    GL_CHECK( glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mpGBufferLightMap->GetTexture(), 0) );
    GL_CHECK( glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, mpGBufferDepth->GetTexture(), 0) );
    GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 }; 
    glDrawBuffers(sizeof(DrawBuffers)/sizeof(DrawBuffers[0]), DrawBuffers);
    auto Completness = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if( Completness != GL_FRAMEBUFFER_COMPLETE )
    {
        GL_CHECK(GL_INVALID_VALUE);
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void ClusteredShadingSample::ReleaseResources()
{
    // Note: these two are defined in the base.  We release them because we addref them.
    SAFE_RELEASE(mpCamera);
    SAFE_RELEASE(mpAssetSet);
    SAFE_RELEASE(mpShadowCamera);
    SAFE_DELETE( mpCameraController );
    SAFE_DELETE( mpDebugSprite);
    SAFE_RELEASE(mpShadowCameraSet);
    SAFE_DELETE( mpShadowRenderTarget );
    SAFE_DELETE( mpScene );
    SAFE_RELEASE( mpCamera );
    SAFE_RELEASE( mpPointLightsInfoTexture );
    SAFE_RELEASE( mpLightGridTexture );
    SAFE_RELEASE( mpUIConstantsBuffer );
    glDeleteFramebuffers(1, &m_GBufferFBO);
    glDeleteVertexArrays(1, &m_DummyVAO);
    SAFE_RELEASE( mpGBufferDiffuseColor );
    SAFE_RELEASE( mpGBufferNormal );
    SAFE_RELEASE( mpGBufferLightMap );
    SAFE_RELEASE( mpGBufferDepth );
	SAFE_RELEASE( mpShadedBackBuffer );
    for(auto It = mpMaterials.begin(); It!=mpMaterials.end(); ++It)
        (*It)->Release();
}

//-----------------------------------------------------------------------------
void ClusteredShadingSample::Update(double deltaSeconds)
{
    mpCameraController->Update((float)deltaSeconds);
    
    if( mpAnimateLightsCheckbox->GetCheckboxState() == CPUT_CHECKBOX_CHECKED )
        Move((float)deltaSeconds);
}

// Handle any control callback events
//-----------------------------------------------------------------------------
void ClusteredShadingSample::HandleGUIElementEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTGUIElement *pElement )
{
}

void ClusteredShadingSample::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{
    switch(ControlID)
    {
        case ID_SHADING_TECH_DROPDOWN:
        {
            int lightCullTech;
            mpShadingTechDropdown->GetSelectedItem(lightCullTech);
            mUIConstants.lightCullTechnique = lightCullTech;
            break;
        }

        case ID_NUM_LIGHTS_SLIDER:
        {
            float fVal;
            ((CPUTSlider*)pControl)->GetValue(fVal);
            SetActiveLights( 1 << (int)fVal );
            #if defined(CPUT_OS_ANDROID)
                std::stringstream ssNumLights;
            #elif defined(CPUT_OS_WINDOWS)
                std::wstringstream ssNumLights;
            #endif
            ssNumLights << _L("Num lights: ") << mActiveLights;
            ((CPUTSlider*)pControl)->SetText(ssNumLights.str());
            break;
        }

        case ID_VISUALIZE_LIGHT_COUNT_CHECKBOX:
            mUIConstants.visualizeLightCount = ((CPUTCheckbox*)pControl)->GetCheckboxState() == CPUT_CHECKBOX_CHECKED;
        break;

        case ID_LIGHTING_ONLY_CHECKBOX:
            mUIConstants.lightingOnly = ((CPUTCheckbox*)pControl)->GetCheckboxState() == CPUT_CHECKBOX_CHECKED;
        break;
    }
}

// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode ClusteredShadingSample::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)
{
    if( mpCameraController )
    {
        if(mpCameraController->HandleMouseEvent(x, y, wheel, state, message) == CPUT_EVENT_HANDLED)
            return CPUT_EVENT_HANDLED;
    }
    
    return CPUT_EVENT_UNHANDLED;
}
