//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 1
//

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include "glfw/glfw3.h"  // Criação de janelas do sistema operacional

#include <array>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static const size_t kCircleSides  = 16U;
static const float  kCircleRadius = 0.7f;

template <typename T>
GLfloat degreesToRadians(T degrees) {
    return static_cast<T>(degrees) * M_PI / 180.0f;
}

constexpr size_t sizeForCircle(size_t sides) noexcept { return 4 * (sides + 1); }

template <size_t sides>
std::array<GLfloat, sizeForCircle(sides)> circleCoefficients(GLfloat radius) {
    std::array<GLfloat, sizeForCircle(sides)> NDC_coefficients{0.0f, 0.0f, 0.0f, 1.0f};

    const GLfloat kIncrementPerIteration = static_cast<GLfloat>(360) / sides;
    for (size_t i = 1; i < sides + 1; ++i) {
        GLfloat current_degrees = static_cast<GLfloat>(i - 1) * kIncrementPerIteration;
        GLfloat current_radians = degreesToRadians(current_degrees);
        // Adjust X
        NDC_coefficients[i * 4 + 0] = std::cos(current_radians) * radius;
        // Adjust Y
        NDC_coefficients[i * 4 + 1] = std::sin(current_radians) * radius;
        // Adjust Z
        NDC_coefficients[i * 4 + 2] = 0.0f;
        // Adjust W
        NDC_coefficients[i * 4 + 3] = 1.0f;
    }

    return NDC_coefficients;
}

template <size_t sides>
std::array<GLfloat, sizeForCircle(sides)> inToOutColored() {
    std::array<GLfloat, sizeForCircle(sides)> colors{1.0f, 0.0f, 0.0f, 1.0f};

    for (size_t i = 1; i < sides + 1; ++i) {
        colors[i * 4 + 0] = 0.0f;  // R
        colors[i * 4 + 1] = 0.0f;  // G
        colors[i * 4 + 2] = 1.0f;  // B
        colors[i * 4 + 3] = 1.0f;  // A
    }

    return colors;
}

template <size_t sides>
std::array<GLubyte, sides + 2> indexesForCircle() {
    std::array<GLubyte, sides + 2> res{};

    std::iota(res.begin(), res.end() - 1, 0);
    res.back() = 1;

    return res;
}

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
GLuint BuildTriangles();        // Constrói triângulos para renderização
void   LoadShadersFromFiles();  // Carrega os shaders de vértice e fragmento,
// criando um programa de GPU
GLuint LoadShader_Vertex(const char* filename);    // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename);  // Carrega um fragment shader
void   LoadShader(const char* filename,
                  GLuint      shader_id);  // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id,
                        GLuint fragment_shader_id);  // Cria um programa de GPU

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

void ErrorCallback(int error, const char* description);

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);

// Variáveis que definem um programa de GPU (shaders). Veja função
// LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;

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

    /* Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    funções modernas de OpenGL. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 500 colunas e 500 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(500, 500, "INF01047 - 00333482 - Eduardo Menges Mattje", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado.
    glfwSetKeyCallback(window, KeyCallback);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte* vendor      = glGetString(GL_VENDOR);
    const GLubyte* renderer    = glGetString(GL_RENDERER);
    const GLubyte* glversion   = glGetString(GL_VERSION);
    const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento
    // Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Construímos a representação de um triângulo
    GLuint vertex_array_object_id = BuildTriangles();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a
    // janela
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de
        // Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima
        glClear(GL_COLOR_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
        // vértices apontados pelo VAO criado pela função
        // BuildTrianglesForDigitZERO(). Veja comentários detalhados dentro da
        // definição de BuildTrianglesForDigitZERO().
        glBindVertexArray(vertex_array_object_id);

        // Pedimos para a GPU rasterizar os vértices apontados pelo VAO como
        // triângulos.
        //
        //                +--- Veja slides 182-188 do documento
        //                Aula_04_Modelagem_Geometrica_3D.pdf. |          +--- O
        //                array "indices[]" contém 6 índices (veja função
        //                BuildTrianglesForDigitZERO()). |          |  +--- Os
        //                índices são do tipo "GLubyte" (8 bits sem sinal) | | |
        //                +--- Vértices começam em indices[0] (veja função
        //                BuildTrianglesForDigitZERO()). |          |  | | V V
        //                V                 V
        glDrawElements(GL_TRIANGLE_FAN, kCircleSides + 2, GL_UNSIGNED_BYTE, nullptr);

        // "Desligamos" o VAO, evitando assim que operações posteriores venham a
        // alterar o mesmo. Isso evita bugs.
        glBindVertexArray(0);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link:
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

    // A posição de cada vértice é definida por coeficientes em "normalized
    // device coordinates" (NDC), onde cada coeficiente está entre -1 e 1.
    // (Veja slides 131-134 e 141-148 do documento
    // Aula_03_Rendering_Pipeline_Grafico.pdf). Nas aulas sobre transformações
    // geométrica veremos como transformar coeficientes em outros sistemas de
    // coordenadas para coeficientes NDC.
    //
    // Note que aqui adicionamos um quarto coeficiente W (igual a 1.0).
    // Conversaremos sobre isso quando tratarmos de coordenadas homogêneas.
    //
    // Este vetor "NDC_coefficients" define a GEOMETRIA (veja slides 103-110 do
    // documento Aula_04_Modelagem_Geometrica_3D.pdf).
    //
    auto NDC_coefficients = circleCoefficients<kCircleSides>(kCircleRadius);

    // Criamos o identificador (ID) de um Vertex Buffer Object (VBO).  Um VBO é
    // um buffer de memória que irá conter os valores de um certo atributo de
    // um conjunto de vértices; por exemplo: posição, cor, normais, coordenadas
    // de textura. Neste exemplo utilizaremos vários VBOs, um para cada tipo de
    // atributo.  Agora criamos um VBO para armazenarmos um atributo: posição
    // (coeficientes NDC definidos acima).
    GLuint VBO_NDC_coefficients_id;
    glGenBuffers(1, &VBO_NDC_coefficients_id);

    // Criamos o identificador (ID) de um Vertex Array Object (VAO). Um VAO
    // contém a definição de vários atributos de um certo conjunto de vértices;
    // isto é, um VAO irá conter ponteiros para vários VBOs.
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);

    // "Ligamos" o VAO ("bind"). Informamos que iremos atualizar o VAO cujo ID
    // está contido na variável "vertex_array_object_id".
    glBindVertexArray(vertex_array_object_id);

    // "Ligamos" o VBO ("bind"). Informamos que o VBO cujo ID está contido na
    // variável VBO_NDC_coefficients_id será modificado a seguir. A
    // constante "GL_ARRAY_BUFFER" informa que esse buffer é de fato um VBO, e
    // irá conter atributos de vértices.
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);

    // Alocamos memória para o VBO "ligado" acima. Como queremos armazenar
    // nesse VBO todos os valores contidos no array "NDC_coefficients", pedimos
    // para alocar um número de bytes exatamente igual ao tamanho ("size")
    // desse array. A constante "GL_STATIC_DRAW" dá uma dica para o driver da
    // GPU sobre como utilizaremos os dados do VBO. Neste caso, estamos dizendo
    // que não pretendemos alterar tais dados (são estáticos: "STATIC"), e
    // também dizemos que tais dados serão utilizados para renderizar ou
    // desenhar ("DRAW").  Pense que:
    //
    //            glBufferData()  ==  malloc() do C  ==  new do C++.
    //
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * NDC_coefficients.

                                   size(),

                 nullptr, GL_STATIC_DRAW);

    // Finalmente, copiamos os valores do array NDC_coefficients para dentro do
    // VBO "ligado" acima.  Pense que:
    //
    //            glBufferSubData()  ==  memcpy() do C.
    //
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * NDC_coefficients.size(), NDC_coefficients.data());

    // Precisamos então informar um índice de "local" ("location"), o qual será
    // utilizado no shader "shader_vertex.glsl" para acessar os valores
    // armazenados no VBO "ligado" acima. Também, informamos a dimensão (número
    // de coeficientes) destes atributos. Como em nosso caso são coordenadas NDC
    // homogêneas, temos quatro coeficientes por vértice (X,Y,Z,W). Isto define
    // um tipo de dado chamado de "vec4" em "shader_vertex.glsl": um vetor com
    // quatro coeficientes. Finalmente, informamos que os dados estão em ponto
    // flutuante com 32 bits (GL_FLOAT).
    // Esta função também informa que o VBO "ligado" acima em glBindBuffer()
    // está dentro do VAO "ligado" acima por glBindVertexArray().
    // Veja
    // https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object
    GLuint location             = 0;  // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);

    // "Ativamos" os atributos. Informamos que os atributos com índice de local
    // definido acima, na variável "location", deve ser utilizado durante o
    // rendering.
    glEnableVertexAttribArray(location);

    // "Desligamos" o VBO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isto evita bugs.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Agora repetimos todos os passos acima para atribuir um novo atributo a
    // cada vértice: uma cor (veja slides 109-112 do documento
    // Aula_03_Rendering_Pipeline_Grafico.pdf e slide 111 do documento
    // Aula_04_Modelagem_Geometrica_3D.pdf). Tal cor é definida como
    // coeficientes RGBA: Red, Green, Blue, Alpha; isto é: Vermelho, Verde,
    // Azul, Alpha (valor de transparência). Conversaremos sobre sistemas de
    // cores nas aulas de Modelos de Iluminação.
    auto   color_coefficients = inToOutColored<kCircleSides>();
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * color_coefficients.

                                   size(),

                 nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(GLfloat) * color_coefficients.

                                      size(),
                    color_coefficients

                        .

                    data()

    );

    location             = 1;  // "(location = 1)" em "shader_vertex.glsl"
    number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vamos então definir dois triângulos utilizando os vértices do array
    // NDC_coefficients. O primeiro triângulo é formado pelos vértices 0,1,2;
    // e o segundo pelos vértices 2,1,3. Note que usaremos o modo de
    // renderização GL_TRIANGLES na chamada glDrawElements() dentro de main().
    // Veja slides 182-188 do documento Aula_04_Modelagem_Geometrica_3D.pdf.
    //
    // Este vetor "indices" define a TOPOLOGIA (veja slides 103-110 do documento
    // Aula_04_Modelagem_Geometrica_3D.pdf).
    //
    auto indices = indexesForCircle<kCircleSides>();

    // Criamos um buffer OpenGL para armazenar os índices acima
    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

    // Alocamos memória para o buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(GLubyte) * indices.

                                   size(),

                 nullptr, GL_STATIC_DRAW);

    // Copiamos os valores do array indices[] para dentro do buffer.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    sizeof(GLubyte) * indices.

                                      size(),
                    indices

                        .

                    data()

    );

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

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader()
// abaixo.
GLuint LoadShader_Vertex(const char* filename) {
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de
// LoadShader() abaixo.
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

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento
// Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles() {
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
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
    //
    GLuint vertex_shader_id   = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if (g_GpuProgramID != 0) glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
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
        auto* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "== End of link log\n";

        delete[] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Retorna o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula (slides 141-148 do
    // documento Aula_03_Rendering_Pipeline_Grafico.pdf).
    glViewport(0, 0, width, height);
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja
// http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    // ===================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função
    // KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT) std::exit(100 + i);
    // ===================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description) {
    (void)fprintf(stderr, "ERROR %i: GLFW: %s\n", error, description);
}

// vim: set spell spelllang=pt_br :
