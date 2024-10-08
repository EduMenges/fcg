//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 2
//
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <map>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include "glfw/glfw3.h"  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers locais, definidos na pasta "include/"
#include "matrices.h"

constexpr float kCameraSpeed = 0.02F;

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
GLuint BuildTriangles();                                    // Constrói triângulos para renderização
GLuint LoadShader_Vertex(const char* filename);             // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename);           // Carrega um fragment shader
void   LoadShader(const char* filename, GLuint shader_id);  // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);  // Cria um programa de GPU

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();

float TextRendering_LineHeight(GLFWwindow* window);

float TextRendering_CharWidth(GLFWwindow* window);

void TextRendering_PrintString(GLFWwindow* window, const std::string& str, float x, float y, float scale = 1.0F);

void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0F);

void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0F);

void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y,
                                            float scale = 1.0F);

void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y,
                                                      float scale = 1.0F);

void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y,
                                                float scale = 1.0F);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model,
                                           glm::vec4 p_model);

void TextRendering_ShowEulerAngles(GLFWwindow* window);

void TextRendering_ShowProjection(GLFWwindow* window);

void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

void ErrorCallback(int error, const char* description);

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject {
    const char* name;            // Nome do objeto
    void*       first_index;     // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTriangles()
    int         num_indices;     // Número de índices do objeto dentro do vetor indices[] definido em BuildTriangles()
    GLenum      rendering_mode;  // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTriangles() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<const char*, SceneObject> g_VirtualScene;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0F;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float angleX_ = 0.0F;
float angleY_ = 0.0F;
float angleZ_ = 0.0F;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.

constexpr float first_g_CameraTheta = -2.45F;  // Ângulo no plano ZX em relação ao eixo Z
constexpr float first_g_CameraPhi   = 0.4F;    // Ângulo em relação ao eixo Y

float cameraTheta_    = first_g_CameraTheta;  // Ângulo no plano ZX em relação ao eixo Z
float cameraPhi_      = first_g_CameraPhi;    // Ângulo em relação ao eixo Y
float cameraDistance_ = 2.5F;                 // Distância da câmera para a origem

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool usePerspectiveProjection_ = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

bool isMovingForward_  = false;
bool isMovingBackward_ = false;
bool isMovingLeft_     = false;
bool isMovingRight_    = false;

static const glm::vec4 kFirstCameraPos = glm::vec4(3.0F, 2.0F, 3.5F, 1.0F);
glm::vec4              cameraPos_      = kFirstCameraPos;

int main() {
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success) {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 800 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 800, "INF01047 - 00333482 - Eduardo Menges Mattje", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowSize(window, 800, 800);  // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte* vendor      = glGetString(GL_VENDOR);
    const GLubyte* renderer    = glGetString(GL_RENDERER);
    const GLubyte* glversion   = glGetString(GL_VERSION);
    const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 176-196 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_0X/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //       |
    //       o-- ...
    //
    GLuint vertex_shader_id   = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Criamos um programa de GPU utilizando os shaders carregados acima
    GLuint program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Construímos a representação de um triângulo
    GLuint vertex_array_object_id = BuildTriangles();

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl".
    GLint model_uniform = glGetUniformLocation(program_id, "model");  // Variável da matriz "model"
    GLint view_uniform  = glGetUniformLocation(program_id, "view");  // Variável da matriz "view" em shader_vertex.glsl
    GLint projection_uniform =
        glGetUniformLocation(program_id,
                             "projection");  // Variável da matriz "projection" em shader_vertex.glsl
    GLint render_as_black_uniform = glGetUniformLocation(program_id,
                                                         "render_as_black");  // Variável booleana em shader_vertex.glsl

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Variáveis auxiliares utilizadas para chamada à função
    // TextRendering_ShowModelViewProjection(), armazenando matrizes 4x4.
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    float r, y, z, x;

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (glfwWindowShouldClose(window) == 0) {
        // printf("\t%d\n", isMovingForward);
        // printf("%d\t%d\t%d\n\n", isMovingLeft, isMovingBackward, isMovingRight);

        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0F, 1.0F, 1.0F, 1.0F);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(program_id);

        // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
        // vértices apontados pelo VAO criado pela função BuildTriangles(). Veja
        // comentários detalhados dentro da definição de BuildTriangles().
        glBindVertexArray(vertex_array_object_id);

        // Computamos a posição da câmera utilizando coordenadas esféricas. As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().

        r = cameraDistance_;
        y = -r * sin(cameraPhi_);
        z = r * cos(cameraPhi_) * cos(cameraTheta_);
        x = r * cos(cameraPhi_) * sin(cameraTheta_);

        glm::vec4 cameraTarget = glm::vec4(x, y, z, 0.0F);

        glm::vec4 genericUp   = glm::vec4(0.0F, 1.0F, 0.0F, 0.0F);
        glm::vec4 cameraRight = crossproduct(genericUp, cameraTarget);

        glm::vec4 cameraUp = crossproduct(cameraTarget, cameraRight);

        if (isMovingForward_) cameraPos_ = cameraPos_ + (cameraTarget * kCameraSpeed);
        if (isMovingBackward_) cameraPos_ = cameraPos_ - (cameraTarget * kCameraSpeed);
        if (isMovingRight_) cameraPos_ = cameraPos_ - (cameraRight * kCameraSpeed);
        if (isMovingLeft_) cameraPos_ = cameraPos_ + (cameraRight * kCameraSpeed);

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento
        // Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(cameraPos_, cameraTarget, cameraUp);

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        constexpr float kNearplane = -0.1F;   // Posição do "near plane"
        constexpr float kFarplane  = -10.0F;  // Posição do "far plane"

        if (usePerspectiveProjection_) {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = M_PI / 3.0F;
            projection          = Matrix_Perspective(field_of_view, g_ScreenRatio, kNearplane, kFarplane);
        } else {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t    = 1.5F * cameraDistance_ / 2.5F;
            float b    = -t;
            float r    = t * g_ScreenRatio;
            float l    = -r;
            projection = Matrix_Orthographic(l, r, b, t, kNearplane, kFarplane);
        }

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        // Vamos desenhar 3 instâncias (cópias) do cubo
        for (int i = 1; i <= 3; ++i) {
            // Cada cópia do cubo possui uma matriz de modelagem independente,
            // já que cada cópia estará em uma posição (rotação, escala, ...)
            // diferente em relação ao espaço global (World Coordinates). Veja
            // slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
            glm::mat4 model;

            if (i == 1) {
                // A primeira cópia do cubo não sofrerá nenhuma transformação
                // de modelagem. Portanto, sua matriz "model" é a identidade, e
                // suas coordenadas no espaço global (World Coordinates) serão
                // *exatamente iguais* a suas coordenadas no espaço do modelo
                // (Model Coordinates).
                model = Matrix_Identity();
            } else if (i == 2) {
                // A segunda cópia do cubo sofrerá um escalamento não-uniforme,
                // seguido de uma rotação no eixo (1,1,1), e uma translação em Z (nessa ordem!).
                model = Matrix_Translate(0.0F, 0.0F, -2.0F)                              // TERCEIRO translação
                        * Matrix_Rotate(M_PI / 8.0F, glm::vec4(1.0F, 1.0F, 1.0F, 0.0F))  // SEGUNDO rotação
                        * Matrix_Scale(2.0F, 0.5F, 0.5F);                                // PRIMEIRO escala
            } else if (i == 3) {
                // A terceira cópia do cubo sofrerá rotações em X,Y e Z (nessa
                // ordem) seguindo o sistema de ângulos de Euler, e após uma
                // translação em X. Veja slides 106-107 do documento Aula_07_Transformacoes_Geometricas_3D.pdf.
                model = Matrix_Translate(-2.0F, 0.0F, 0.0F)  // QUARTO translação
                        * Matrix_Rotate_Z(angleZ_)           // TERCEIRO rotação Z de Euler
                        * Matrix_Rotate_Y(angleY_)           // SEGUNDO rotação Y de Euler
                        * Matrix_Rotate_X(angleX_);          // PRIMEIRO rotação X de Euler

                // Armazenamos as matrizes model, view, e projection do terceiro cubo
                // para mostrar elas na tela através da função TextRendering_ShowModelViewProjection().
                the_model      = model;
                the_projection = projection;
                the_view       = view;
            }

            // Enviamos a matriz "model" para a placa de vídeo (GPU). Veja o
            // arquivo "shader_vertex.glsl", onde esta é efetivamente
            // aplicada em todos os pontos.
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

            // Informamos para a placa de vídeo (GPU) que a variável booleana
            // "render_as_black" deve ser colocada como "false". Veja o arquivo
            // "shader_vertex.glsl".
            glUniform1i(render_as_black_uniform, GL_FALSE);

            // Pedimos para a GPU rasterizar os vértices do cubo apontados pelo
            // VAO como triângulos, formando as faces do cubo. Esta
            // renderização irá executar o Vertex Shader definido no arquivo
            // "shader_vertex.glsl", e o mesmo irá utilizar as matrizes
            // "model", "view" e "projection" definidas acima e já enviadas
            // para a placa de vídeo (GPU).
            //
            // Veja a definição de g_VirtualScene["cube_faces"] dentro da
            // função BuildTriangles(), e veja a documentação da função
            // glDrawElements() em http://docs.gl/gl3/glDrawElements.
            glDrawElements(g_VirtualScene["cube_faces"]
                               .rendering_mode,  // Veja slides 124-130 do documento Aula_04_Modelagem_Geometrica_3D.pdf
                           g_VirtualScene["cube_faces"].num_indices, GL_UNSIGNED_INT,
                           g_VirtualScene["cube_faces"].first_index);

            // Pedimos para OpenGL desenhar linhas com largura de 4 pixels.
            glLineWidth(4.0F);

            // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
            // apontados pelo VAO como linhas. Veja a definição de
            // g_VirtualScene["axes"] dentro da função BuildTriangles(), e veja
            // a documentação da função glDrawElements() em
            // http://docs.gl/gl3/glDrawElements.
            //
            // Importante: estes eixos serão desenhamos com a matriz "model"
            // definida acima, e portanto sofrerão as mesmas transformações
            // geométricas que o cubo. Isto é, estes eixos estarão
            // representando o sistema de coordenadas do modelo (e não o global)!
            glDrawElements(g_VirtualScene["axes"].rendering_mode, g_VirtualScene["axes"].num_indices, GL_UNSIGNED_INT,
                           g_VirtualScene["axes"].first_index);

            // Informamos para a placa de vídeo (GPU) que a variável booleana
            // "render_as_black" deve ser colocada como "true". Veja o arquivo
            // "shader_vertex.glsl".
            glUniform1i(render_as_black_uniform, GL_TRUE);

            // Pedimos para a GPU rasterizar os vértices do cubo apontados pelo
            // VAO como linhas, formando as arestas pretas do cubo. Veja a
            // definição de g_VirtualScene["cube_edges"] dentro da função
            // BuildTriangles(), e veja a documentação da função
            // glDrawElements() em http://docs.gl/gl3/glDrawElements.
            glDrawElements(g_VirtualScene["cube_edges"].rendering_mode, g_VirtualScene["cube_edges"].num_indices,
                           GL_UNSIGNED_INT, g_VirtualScene["cube_edges"].first_index);

            // Desenhamos um ponto de tamanho 15 pixels em cima do terceiro vértice
            // do terceiro cubo. Este vértice tem coordenada de modelo igual à
            // (0.5, 0.5, 0.5, 1.0).
            if (i == 3) {
                glPointSize(15.0F);
                glDrawArrays(GL_POINTS, 3, 1);
            }
        }

        // Agora queremos desenhar os eixos XYZ de coordenadas GLOBAIS.
        // Para tanto, colocamos a matriz de modelagem igual à identidade.
        // Veja slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 model = Matrix_Identity();

        // Enviamos a nova matriz "model" para a placa de vídeo (GPU). Veja o
        // arquivo "shader_vertex.glsl".
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

        // Pedimos para OpenGL desenhar linhas com largura de 10 pixels.
        glLineWidth(10.0F);

        // Informamos para a placa de vídeo (GPU) que a variável booleana
        // "render_as_black" deve ser colocada como "false". Veja o arquivo
        // "shader_vertex.glsl".
        glUniform1i(render_as_black_uniform, GL_FALSE);

        // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
        // apontados pelo VAO como linhas. Veja a definição de
        // g_VirtualScene["axes"] dentro da função BuildTriangles(), e veja
        // a documentação da função glDrawElements() em
        // http://docs.gl/gl3/glDrawElements.
        glDrawElements(g_VirtualScene["axes"].rendering_mode, g_VirtualScene["axes"].num_indices, GL_UNSIGNED_INT,
                       g_VirtualScene["axes"].first_index);

        // "Desligamos" o VAO, evitando assim que operações posteriores venham a
        // alterar o mesmo. Isso evita bugs.
        glBindVertexArray(0);

        // Pegamos um vértice com coordenadas de modelo (0.5, 0.5, 0.5, 1) e o
        // passamos por todos os sistemas de coordenadas armazenados nas
        // matrizes the_model, the_view, e the_projection; e escrevemos na tela
        // as matrizes e pontos resultantes dessas transformações.
        glm::vec4 p_model(0.5F, 0.5F, 0.5F, 1.0F);
        TextRendering_ShowModelViewProjection(window, the_projection, the_view, the_model, p_model);

        // Imprimimos na tela os ângulos de Euler que controlam a rotação do
        // terceiro cubo.
        TextRendering_ShowEulerAngles(window);

        // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
        TextRendering_ShowProjection(window);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: Veja o link:
        // https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Constrói triângulos para futura renderização
GLuint BuildTriangles() {
    // Primeiro, definimos os atributos de cada vértice.

    // A posição de cada vértice é definida por coeficientes em um sistema de
    // coordenadas local de cada modelo geométrico. Note o uso de coordenadas
    // homogêneas.  Veja as seguintes referências:
    //
    //  - slides 35-48 do documento Aula_08_Sistemas_de_Coordenadas.pdf;
    //  - slides 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf;
    //
    // Este vetor "model_coefficients" define a GEOMETRIA (veja slides 64-71 do documento
    // Aula_04_Modelagem_Geometrica_3D.pdf).
    //
    GLfloat model_coefficients[] = {
        // Vértices de um cubo
        //    X      Y     Z     W
        -0.5F, 0.5F, 0.5F,
        1.0F,  // posição do vértice 0
        -0.5F, -0.5F, 0.5F,
        1.0F,  // posição do vértice 1
        0.5F, -0.5F, 0.5F,
        1.0F,  // posição do vértice 2
        0.5F, 0.5F, 0.5F,
        1.0F,  // posição do vértice 3
        -0.5F, 0.5F, -0.5F,
        1.0F,  // posição do vértice 4
        -0.5F, -0.5F, -0.5F,
        1.0F,  // posição do vértice 5
        0.5F, -0.5F, -0.5F,
        1.0F,  // posição do vértice 6
        0.5F, 0.5F, -0.5F,
        1.0F,  // posição do vértice 7
        // Vértices para desenhar o eixo X
        //    X      Y     Z     W
        0.0F, 0.0F, 0.0F,
        1.0F,  // posição do vértice 8
        1.0F, 0.0F, 0.0F,
        1.0F,  // posição do vértice 9
        // Vértices para desenhar o eixo Y
        //    X      Y     Z     W
        0.0F, 0.0F, 0.0F,
        1.0F,  // posição do vértice 10
        0.0F, 1.0F, 0.0F,
        1.0F,  // posição do vértice 11
        // Vértices para desenhar o eixo Z
        //    X      Y     Z     W
        0.0F, 0.0F, 0.0F,
        1.0F,  // posição do vértice 12
        0.0F, 0.0F, 1.0F,
        1.0F,  // posição do vértice 13
    };

    // Criamos o identificador (ID) de um Vertex Buffer Object (VBO).  Um VBO é
    // um buffer de memória que irá conter os valores de um certo atributo de
    // um conjunto de vértices; por exemplo: posição, cor, normais, coordenadas
    // de textura.  Neste exemplo utilizaremos vários VBOs, um para cada tipo de atributo.
    // Agora criamos um VBO para armazenarmos um atributo: posição.
    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);

    // Criamos o identificador (ID) de um Vertex Array Object (VAO).  Um VAO
    // contém a definição de vários atributos de um certo conjunto de vértices;
    // isto é, um VAO irá conter ponteiros para vários VBOs.
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);

    // "Ligamos" o VAO ("bind"). Informamos que iremos atualizar o VAO cujo ID
    // está contido na variável "vertex_array_object_id".
    glBindVertexArray(vertex_array_object_id);

    // "Ligamos" o VBO ("bind"). Informamos que o VBO cujo ID está contido na
    // variável VBO_model_coefficients_id será modificado a seguir. A
    // constante "GL_ARRAY_BUFFER" informa que esse buffer é de fato um VBO, e
    // irá conter atributos de vértices.
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);

    // Alocamos memória para o VBO "ligado" acima. Como queremos armazenar
    // nesse VBO todos os valores contidos no array "model_coefficients", pedimos
    // para alocar um número de bytes exatamente igual ao tamanho ("size")
    // desse array. A constante "GL_STATIC_DRAW" dá uma dica para o driver da
    // GPU sobre como utilizaremos os dados do VBO. Neste caso, estamos dizendo
    // que não pretendemos alterar tais dados (são estáticos: "STATIC"), e
    // também dizemos que tais dados serão utilizados para renderizar ou
    // desenhar ("DRAW").  Pense que:
    //
    //            glBufferData()  ==  malloc() do C  ==  new do C++.
    //
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), nullptr, GL_STATIC_DRAW);

    // Finalmente, copiamos os valores do array model_coefficients para dentro do
    // VBO "ligado" acima.  Pense que:
    //
    //            glBufferSubData()  ==  memcpy() do C.
    //
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);

    // Precisamos então informar um índice de "local" ("location"), o qual será
    // utilizado no shader "shader_vertex.glsl" para acessar os valores
    // armazenados no VBO "ligado" acima. Também, informamos a dimensão (número de
    // coeficientes) destes atributos. Como em nosso caso são pontos em coordenadas
    // homogêneas, temos quatro coeficientes por vértice (X,Y,Z,W). Isso define
    // um tipo de dado chamado de "vec4" em "shader_vertex.glsl": um vetor com
    // quatro coeficientes. Finalmente, informamos que os dados estão em ponto
    // flutuante com 32 bits (GL_FLOAT).
    // Esta função também informa que o VBO "ligado" acima em glBindBuffer()
    // está dentro do VAO "ligado" acima por glBindVertexArray().
    // Veja https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object
    GLuint location             = 0;  // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

    // "Ativamos" os atributos. Informamos que os atributos com índice de local
    // definido acima, na variável "location", deve ser utilizado durante o
    // rendering.
    glEnableVertexAttribArray(location);

    // "Desligamos" o VBO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Agora repetimos todos os passos acima para atribuir um novo atributo a
    // cada vértice: uma cor (veja slides 107-110 do documento Aula_03_Rendering_Pipeline_Grafico.pdf e slide 72 do
    // documento Aula_04_Modelagem_Geometrica_3D.pdf). Tal cor é definida como coeficientes RGBA: Red, Green, Blue,
    // Alpha; isto é: Vermelho, Verde, Azul, Alpha (valor de transparência). Conversaremos sobre sistemas de cores nas
    // aulas de Modelos de Iluminação.
    GLfloat color_coefficients[] = {
        // Cores dos vértices do cubo
        //  R     G     B     A
        1.0F, 0.5F, 0.0F,
        1.0F,  // cor do vértice 0
        1.0F, 0.5F, 0.0F,
        1.0F,  // cor do vértice 1
        0.0F, 0.5F, 1.0F,
        1.0F,  // cor do vértice 2
        0.0F, 0.5F, 1.0F,
        1.0F,  // cor do vértice 3
        1.0F, 0.5F, 0.0F,
        1.0F,  // cor do vértice 4
        1.0F, 0.5F, 0.0F,
        1.0F,  // cor do vértice 5
        0.0F, 0.5F, 1.0F,
        1.0F,  // cor do vértice 6
        0.0F, 0.5F, 1.0F,
        1.0F,  // cor do vértice 7
        // Cores para desenhar o eixo X
        1.0F, 0.0F, 0.0F,
        1.0F,  // cor do vértice 8
        1.0F, 0.0F, 0.0F,
        1.0F,  // cor do vértice 9
        // Cores para desenhar o eixo Y
        0.0F, 1.0F, 0.0F,
        1.0F,  // cor do vértice 10
        0.0F, 1.0F, 0.0F,
        1.0F,  // cor do vértice 11
        // Cores para desenhar o eixo Z
        0.0F, 0.0F, 1.0F,
        1.0F,  // cor do vértice 12
        0.0F, 0.0F, 1.0F,
        1.0F,  // cor do vértice 13
    };
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location             = 1;  // "(location = 1)" em "shader_vertex.glsl"
    number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vamos então definir polígonos utilizando os vértices do array
    // model_coefficients.
    //
    // Para referência sobre os modos de renderização, veja slides 124-130 do documento
    // Aula_04_Modelagem_Geometrica_3D.pdf.
    //
    // Este vetor "indices" define a TOPOLOGIA (veja slides 64-71 do documento Aula_04_Modelagem_Geometrica_3D.pdf).
    //
    GLuint indices[] = {
        // Definimos os índices dos vértices que definem as FACES de um cubo
        // através de 12 triângulos que serão desenhados com o modo de renderização
        // GL_TRIANGLES.
        0, 1,
        2,  // triângulo 1
        7, 6,
        5,  // triângulo 2
        3, 2,
        6,  // triângulo 3
        4, 0,
        3,  // triângulo 4
        4, 5,
        1,  // triângulo 5
        1, 5,
        6,  // triângulo 6
        0, 2,
        3,  // triângulo 7
        7, 5,
        4,  // triângulo 8
        3, 6,
        7,  // triângulo 9
        4, 3,
        7,  // triângulo 10
        4, 1,
        0,  // triângulo 11
        1, 6,
        2,  // triângulo 12
        // Definimos os índices dos vértices que definem as ARESTAS de um cubo
        // através de 12 linhas que serão desenhadas com o modo de renderização
        // GL_LINES.
        0,
        1,  // linha 1
        1,
        2,  // linha 2
        2,
        3,  // linha 3
        3,
        0,  // linha 4
        0,
        4,  // linha 5
        4,
        7,  // linha 6
        7,
        6,  // linha 7
        6,
        2,  // linha 8
        6,
        5,  // linha 9
        5,
        4,  // linha 10
        5,
        1,  // linha 11
        7,
        3,  // linha 12
        // Definimos os índices dos vértices que definem as linhas dos eixos X, Y,
        // Z, que serão desenhados com o modo GL_LINES.
        8,
        9,  // linha 1
        10,
        11,  // linha 2
        12,
        13  // linha 3
    };

    // Criamos um primeiro objeto virtual (SceneObject) que se refere às faces
    // coloridas do cubo.
    SceneObject cube_faces{};
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = reinterpret_cast<void*>(0);  // Primeiro índice está em indices[0]
    cube_faces.num_indices    = 36;            // Último índice está em indices[35]; total de 36 índices.
    cube_faces.rendering_mode = GL_TRIANGLES;  // Índices correspondem ao tipo de rasterização GL_TRIANGLES.

    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["cube_faces"] = cube_faces;

    // Criamos um segundo objeto virtual (SceneObject) que se refere às arestas
    // pretas do cubo.
    SceneObject cube_edges{};
    cube_edges.name           = "Cubo (arestas pretas)";
    cube_edges.first_index    = reinterpret_cast<void*>(36 * sizeof(GLuint));  // Primeiro índice está em indices[36]
    cube_edges.num_indices    = 24;        // Último índice está em indices[59]; total de 24 índices.
    cube_edges.rendering_mode = GL_LINES;  // Índices correspondem ao tipo de rasterização GL_LINES.

    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["cube_edges"] = cube_edges;

    // Criamos um terceiro objeto virtual (SceneObject) que se refere aos eixos XYZ.
    SceneObject axes{};
    axes.name              = "Eixos XYZ";
    axes.first_index       = reinterpret_cast<void*>(60 * sizeof(GLuint));  // Primeiro índice está em indices[60]
    axes.num_indices       = 6;         // Último índice está em indices[65]; total de 6 índices.
    axes.rendering_mode    = GL_LINES;  // Índices correspondem ao tipo de rasterização GL_LINES.
    g_VirtualScene["axes"] = axes;

    // Criamos um buffer OpenGL para armazenar os índices acima
    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

    // Alocamos memória para o buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), nullptr, GL_STATIC_DRAW);

    // Copiamos os valores do array indices[] para dentro do buffer.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    // NÃO faça a chamada abaixo! Diferente de um VBO (GL_ARRAY_BUFFER), um
    // array de índices (GL_ELEMENT_ARRAY_BUFFER) não pode ser "desligado",
    // caso contrário o VAO irá perder a informação sobre os índices.
    //
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);

    // Retornamos o ID do VAO. Isso é tudo que será necessário para renderizar
    // os triângulos definidos acima. Veja a chamada glDrawElements() em main().
    return vertex_array_object_id;
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename) {
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename) {
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id) {
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch (std::exception& e) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string   str                  = shader.str();
    const GLchar* shader_string        = str.c_str();
    const auto    shader_string_length = static_cast<GLint>(str.length());

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    auto* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if (log_length != 0) {
        std::string output;

        if (!compiled_ok) {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        } else {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete[] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id) {
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if (linked_ok == GL_FALSE) {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        auto* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete[] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (!g_LeftMouseButtonPressed) {
        return;
    }

    // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    // Atualizamos parâmetros da câmera com os deslocamentos
    cameraTheta_ -= 0.01F * dx;
    cameraPhi_ += 0.01F * dy;

    // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
    float phimax = M_PI / 2;
    float phimin = -phimax;

    if (cameraPhi_ > phimax) {
        cameraPhi_ = phimax;
    }

    if (cameraPhi_ < phimin) {
        cameraPhi_ = phimin;
    }

    // Atualizamos as variáveis globais para armazenar a posição atual do
    // cursor como sendo a última posição conhecida do cursor.
    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    cameraDistance_ -= static_cast<float>(0.1 * yoffset);

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (cameraDistance_ < verysmallnumber) cameraDistance_ = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    // ==============
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i) {
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT) {
            std::exit(100 + i);
        }
    }
    // ==============

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    float kDelta = M_PI / 16.0F;  // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        angleX_ += (mod & GLFW_MOD_SHIFT) != 0 ? -kDelta : kDelta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        angleY_ += (mod & GLFW_MOD_SHIFT) != 0 ? -kDelta : kDelta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        angleZ_ += (mod & GLFW_MOD_SHIFT) != 0 ? -kDelta : kDelta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        angleX_ = 0.0F;
        angleY_ = 0.0F;
        angleZ_ = 0.0F;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        usePerspectiveProjection_ = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        usePerspectiveProjection_ = false;
    }

    // Forward
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            isMovingForward_ = true;
        } else if (action == GLFW_RELEASE) {
            isMovingForward_ = false;
        }
    }

    // Backward
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) {
            isMovingBackward_ = true;
        } else if (action == GLFW_RELEASE) {
            isMovingBackward_ = false;
        }
    }

    // Left
    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            isMovingLeft_ = true;
        } else if (action == GLFW_RELEASE) {
            isMovingLeft_ = false;
        }
    }

    // Right
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            isMovingRight_ = true;
        } else if (action == GLFW_RELEASE) {
            isMovingRight_ = false;
        }
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        cameraPos_   = kFirstCameraPos;
        cameraPhi_   = first_g_CameraPhi;
        cameraTheta_ = first_g_CameraTheta;
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description) { fprintf(stderr, "ERROR: GLFW: %s\n", description); }

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model,
                                           glm::vec4 p_model) {
    if (!g_ShowInfoText) return;

    glm::vec4 p_world  = model * p_model;
    glm::vec4 p_camera = view * p_world;
    glm::vec4 p_clip   = projection * p_camera;
    glm::vec4 p_ndc    = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0F, 1.0F - pad, 1.0F);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0F, 1.0F - 2 * pad, 1.0F);

    TextRendering_PrintString(window, "                                        |  ", -1.0F, 1.0F - 6 * pad, 1.0F);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0F, 1.0F - 7 * pad, 1.0F);
    TextRendering_PrintString(window, "                            V              ", -1.0F, 1.0F - 8 * pad, 1.0F);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0F, 1.0F - 9 * pad,
                              1.0F);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0F, 1.0F - 10 * pad, 1.0F);

    TextRendering_PrintString(window, "                                        |  ", -1.0F, 1.0F - 14 * pad, 1.0F);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0F, 1.0F - 15 * pad, 1.0F);
    TextRendering_PrintString(window, "                            V              ", -1.0F, 1.0F - 16 * pad, 1.0F);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0F,
                              1.0F - 17 * pad, 1.0F);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0F, 1.0F - 18 * pad, 1.0F);

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2(0, 0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix((q.x - p.x) / (b.x - a.x), 0.0F, 0.0F, (b.x * p.x - a.x * q.x) / (b.x - a.x),
                                        0.0F, (q.y - p.y) / (b.y - a.y), 0.0F, (b.y * p.y - a.y * q.y) / (b.y - a.y),
                                        0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F);

    TextRendering_PrintString(window, "                                                       |  ", -1.0F,
                              1.0F - 22 * pad, 1.0F);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0F,
                              1.0F - 23 * pad, 1.0F);
    TextRendering_PrintString(window, "                            V                           ", -1.0F,
                              1.0F - 24 * pad, 1.0F);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0F, 1.0F - 25 * pad,
                              1.0F);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0F, 1.0F - 26 * pad, 1.0F);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window) {
    if (!g_ShowInfoText) {
        return;
    }

    float            pad         = TextRendering_LineHeight(window);
    constexpr size_t kBufferSize = 80;
    char             buffer[kBufferSize];

    snprintf(buffer, kBufferSize, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", angleZ_, angleY_,
             angleX_);

    TextRendering_PrintString(window, buffer, -1.0F + pad / 10, -1.0F + 2 * pad / 10, 1.0F);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window) {
    if (!g_ShowInfoText) {
        return;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth  = TextRendering_CharWidth(window);

    if (usePerspectiveProjection_) {
        TextRendering_PrintString(window, "Perspective", 1.0F - 13 * charwidth, -1.0F + 2 * lineheight / 10, 1.0F);
    } else {
        TextRendering_PrintString(window, "Orthographic", 1.0F - 13 * charwidth, -1.0F + 2 * lineheight / 10, 1.0F);
    }
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window) {
    if (!g_ShowInfoText) {
        return;
    }

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static auto old_seconds     = static_cast<float>(glfwGetTime());
    static int  ellapsed_frames = 0;
    static char buffer[20]      = "?? fps";
    static int  numchars        = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    auto seconds = static_cast<float>(glfwGetTime());

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if (ellapsed_seconds > 1.0F) {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds     = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth  = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0F - (numchars + 1) * charwidth, 1.0F - lineheight, 1.0F);
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :
