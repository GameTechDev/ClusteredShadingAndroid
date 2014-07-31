
[DepthStencilStateOGL]
DepthEnable = false

[RasterizerStateOGL]
DepthClipEnable = true
ScissorEnable = false
MultisampleEnable = false
AntialiasedLineEnable = false

[RenderTargetBlendStateOGL]
blendenable = true
srcblend = GL_SRC_ALPHA
destblend = GL_ONE_MINUS_SRC_ALPHA
srcblendalpha = GL_ONE
destblendalpha = GL_ONE
blendop = gl_func_add
blendopalpha = gl_func_add


[SamplerOGL_1]
MinFilter = GL_NEAREST
MagFilter = GL_NEAREST
AddressU = GL_CLAMP_TO_EDGE
AddressV = GL_CLAMP_TO_EDGE
AddressW = GL_CLAMP_TO_EDGE
