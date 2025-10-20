static string OpenGLOutputVertexShader = R"(
  #version 150

  uniform vec4 targetSize;
  uniform vec4 outputSize;
  uniform mat4 cameraModelViewProjection;
  uniform int cameraEnabled;

  in vec2 texCoord;

  out Vertex {
    vec2 texCoord;
  } vertexOut;

  void main() {
    vec2 base;
    base.x = (gl_VertexID == 0 || gl_VertexID == 2 ? -targetSize.x : targetSize.x) / outputSize.x;
    base.y = (gl_VertexID == 0 || gl_VertexID == 1 ?  targetSize.y : -targetSize.y) / outputSize.y;
    vec4 position = vec4(base, 0.0, 1.0);

    if(cameraEnabled != 0) {
      position = cameraModelViewProjection * position;
    } else {
      vec2 align = fract((outputSize.xy + targetSize.xy) / 2.0) * 2.0;
      position.xy -= align / outputSize.xy;
    }

    gl_Position = position;

    vertexOut.texCoord = texCoord;
  }
)";

static string OpenGLVertexShader = R"(
  #version 150

  in vec4 position;
  in vec2 texCoord;

  out Vertex {
    vec2 texCoord;
  } vertexOut;

  void main() {
    gl_Position = position;
    vertexOut.texCoord = texCoord;
  }
)";

static string OpenGLGeometryShader = R"(
  #version 150

  layout(triangles) in;
  layout(triangle_strip, max_vertices = 3) out;

  in Vertex {
    vec2 texCoord;
  } vertexIn[];

  out Vertex {
    vec2 texCoord;
  };

  void main() {
    for(int i = 0; i < gl_in.length(); i++) {
      gl_Position = gl_in[i].gl_Position;
      texCoord = vertexIn[i].texCoord;
      EmitVertex();
    }
    EndPrimitive();
  }
)";

static string OpenGLFragmentShader = R"(
  #version 150

  uniform sampler2D source[];

  in Vertex {
    vec2 texCoord;
  };

  out vec4 fragColor;

  void main() {
    fragColor = texture(source[0], texCoord);
  }
)";
