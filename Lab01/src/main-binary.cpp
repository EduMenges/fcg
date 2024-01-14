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

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static const size_t kZeroSides = 16U;

template <typename T, size_t N>
constexpr size_t totalSize(const std::array<T, N>& collection) {
    return collection.size() * sizeof(T);
}

template <typename T>
constexpr GLfloat degreesToRadians(T degrees) noexcept {
    return static_cast<T>(degrees) * M_PI / 180.0f;
}

constexpr size_t sizeForCircle(size_t sides) noexcept { return 4 * sides; }

constexpr GLfloat kHeight      = 0.3f;
constexpr GLfloat kSpacing     = 0.5f;
constexpr size_t  kDigitAmount = 4;

namespace zero {

constexpr size_t size(size_t sides) noexcept { return 4 * sides * 2; }

constexpr size_t indexAmount(size_t sides) noexcept { return (sides + 1) * 2; }

template <size_t sides>
std::array<GLfloat, sizeForCircle(sides)> ellipseCoefficients(GLfloat radius_h, GLfloat radius_v, GLfloat c_x = 0.0f,
                                                              GLfloat c_y = 0.0f) {
    std::array<GLfloat, sizeForCircle(sides)> NDC_coefficients{};

    const GLfloat kIncrementPerIteration = static_cast<GLfloat>(360) / sides;
    for (size_t i = 0; i < sides; ++i) {
        GLfloat current_degrees = static_cast<GLfloat>(i) * kIncrementPerIteration;
        GLfloat current_radians = degreesToRadians(current_degrees);
        // Adjust X
        NDC_coefficients[i * 4 + 0] = std::cos(current_radians) * radius_h + c_x;
        // Adjust Y
        NDC_coefficients[i * 4 + 1] = std::sin(current_radians) * radius_v + c_y;
        // Adjust Z
        NDC_coefficients[i * 4 + 2] = 0.0f;
        // Adjust W
        NDC_coefficients[i * 4 + 3] = 1.0f;
    }

    return NDC_coefficients;
}

template <size_t sides>
std::array<GLfloat, size(sides)> coefficients(GLfloat c_x = 0.0f, GLfloat c_y = 0.0f) {
    constexpr float                  kThickness = 0.05f;
    const float                      kLength    = kHeight * 0.7f;
    std::array<GLfloat, size(sides)> result{};

    std::array<GLfloat, sizeForCircle(sides)> externalCircle = ellipseCoefficients<sides>(kLength, kHeight, c_x, c_y);
    std::array<GLfloat, sizeForCircle(sides)> internalCircle =
        ellipseCoefficients<sides>(kLength - kThickness, kHeight - kThickness, c_x, c_y);

    std::copy(externalCircle.cbegin(), externalCircle.cend(), result.begin());
    std::copy(internalCircle.cbegin(), internalCircle.cend(), result.begin() + sizeForCircle(sides));

    return result;
}

template <size_t sides>
std::array<GLfloat, size(sides)> colors() {
    std::array<GLfloat, size(sides)> colors{};

    for (size_t i = 0; i < sides * 2; ++i) {
        colors[i * 4 + 0] = 1.0f;  // R
        colors[i * 4 + 1] = 0.0f;  // G
        colors[i * 4 + 2] = 0.0f;  // B
        colors[i * 4 + 3] = 1.0f;  // A
    }

    return colors;
}

template <size_t sides>
std::array<GLubyte, indexAmount(sides)> indexes() {
    std::array<GLubyte, indexAmount(sides)> res{};

    GLubyte j = 0;
    size_t  i = 0;

    for (; i < (sides)*2; i += 2, ++j) {
        res[i]     = j;
        res[i + 1] = static_cast<GLubyte>(sides + j);
    }

    res[i]     = 0;
    res[i + 1] = sides;

    return res;
}
}

namespace one {
constexpr size_t indexAmount() noexcept { return 6; }

constexpr size_t size() noexcept { return 4 * indexAmount(); }

std::array<GLfloat, size()> colors() {
    std::array<GLfloat, size()> colors{};

    for (size_t i = 0; i < indexAmount(); ++i) {
        colors[i * 4 + 0] = 0.0f;  // R
        colors[i * 4 + 1] = 0.0f;  // G
        colors[i * 4 + 2] = 1.0f;  // B
        colors[i * 4 + 3] = 1.0f;  // A
    }

    return colors;
}

std::array<GLfloat, size()> coefficients(GLfloat c_x = 0.0f, GLfloat c_y = 0.0f) {
    const float kLength = kHeight * 0.2f;

    std::array<GLfloat, size()> cu = {
        -kLength * 0.3f + c_x, -kHeight + c_y,       0.0f, 1.0f,  // 0
        kLength * 0.7f + c_x,  -kHeight + c_y,       0.0f, 1.0f,  // 1
        -kLength * 0.3f + c_x, kHeight * 0.7f + c_y, 0.0f, 1.0f,  // 2
        kLength * 0.7f + c_x,  kHeight + c_y,        0.0f, 1.0f,  // 3
        -kLength * 0.3f + c_x, kHeight + c_y,        0.0f, 1.0f,  // 4
        -kLength * 2.0f + c_x, kHeight * 0.5f + c_y, 0.0f, 1.0f   // 5
    };

    return cu;
}

std::array<GLubyte, indexAmount()> indexes() { return {0, 1, 2, 3, 4, 5}; }
}

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
std::array<GLuint, kDigitAmount> BuildTrianglesForDigitZERO();  // Constrói triângulos para renderização
std::array<GLuint, kDigitAmount> BuildTrianglesForDigitONE();   // Constrói triângulos para renderização
void                             LoadShadersFromFiles();        // Carrega os shaders de vértice e fragmento,
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
GLuint id_ = 0;

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
    window = glfwCreateWindow(1000, 1000, "INF01047 - 00333482 - Eduardo Menges Mattje", nullptr, nullptr);
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

    auto VAO_zeros = BuildTrianglesForDigitZERO();
    auto VAO_ones  = BuildTrianglesForDigitONE();

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a
    // janela
    while (!glfwWindowShouldClose(window)) {
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
        glUseProgram(id_);

        auto seconds = static_cast<uint8_t>(glfwGetTime());

        for (int i = kDigitAmount - 1; i >= 0; --i) {
            if ((seconds & (0x01 << (kDigitAmount - i - 1))) != 0) {
                glBindVertexArray(VAO_ones[i]);
                glDrawElements(GL_TRIANGLE_STRIP, one::indexAmount(), GL_UNSIGNED_BYTE, nullptr);
            } else {
                glBindVertexArray(VAO_zeros[i]);
                glDrawElements(GL_TRIANGLE_STRIP, zero::indexAmount(kZeroSides), GL_UNSIGNED_BYTE, nullptr);
            }

            glBindVertexArray(0);
        }

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
std::array<GLuint, kDigitAmount> BuildTrianglesForDigitZERO() {
    std::array<GLuint, kDigitAmount> ret{};

    // Criamos o identificador (ID) de um Vertex Array Object (VAO). Um VAO
    // contém a definição de vários atributos de um certo conjunto de vértices;
    // isto é, um VAO irá conter ponteiros para vários VBOs.
    glGenVertexArrays(kDigitAmount, ret.data());

    GLfloat start_x = -0.75f;

    for (GLuint& vertex_array_object_id : ret) {
        auto NDC_coefficients = zero::coefficients<kZeroSides>(start_x);

        GLuint VBO_NDC_coefficients_id;
        glGenBuffers(1, &VBO_NDC_coefficients_id);

        glBindVertexArray(vertex_array_object_id);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);

        glBufferData(GL_ARRAY_BUFFER, totalSize(NDC_coefficients), nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, totalSize(NDC_coefficients), NDC_coefficients.data());

        GLuint location             = 0;  // "(location = 0)" em "shader_vertex.glsl"
        GLint  number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(location);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        auto   color_coefficients = zero::colors<kZeroSides>();
        GLuint VBO_color_coefficients_id;
        glGenBuffers(1, &VBO_color_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, totalSize(color_coefficients), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, totalSize(color_coefficients), color_coefficients.data());

        location             = 1;  // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        auto indices = zero::indexes<kZeroSides>();

        GLuint indices_id;
        glGenBuffers(1, &indices_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalSize(indices), nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, totalSize(indices), indices.data());

        glBindVertexArray(0);

        start_x += kSpacing;
    }

    return ret;
}

// Constrói triângulos para futura renderização
std::array<GLuint, kDigitAmount> BuildTrianglesForDigitONE() {
    std::array<GLuint, kDigitAmount> ret{};

    // Criamos o identificador (ID) de um Vertex Array Object (VAO). Um VAO
    // contém a definição de vários atributos de um certo conjunto de vértices;
    // isto é, um VAO irá conter ponteiros para vários VBOs.
    glGenVertexArrays(kDigitAmount, ret.data());

    GLfloat start_x = -0.75f;

    for (GLuint& vertex_array_object_id : ret) {
        auto NDC_coefficients = one::coefficients(start_x);

        GLuint VBO_NDC_coefficients_id;
        glGenBuffers(1, &VBO_NDC_coefficients_id);

        glBindVertexArray(vertex_array_object_id);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);

        glBufferData(GL_ARRAY_BUFFER, totalSize(NDC_coefficients), nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, totalSize(NDC_coefficients), NDC_coefficients.data());

        GLuint location             = 0;  // "(location = 0)" em "shader_vertex.glsl"
        GLint  number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);

        glEnableVertexAttribArray(location);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        auto   color_coefficients = one::colors();
        GLuint VBO_color_coefficients_id;
        glGenBuffers(1, &VBO_color_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, totalSize(color_coefficients), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, totalSize(color_coefficients), color_coefficients.data());

        location             = 1;  // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4;  // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        auto indices = one::indexes();

        GLuint indices_id;
        glGenBuffers(1, &indices_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalSize(indices), nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, totalSize(indices), indices.data());

        glBindVertexArray(0);

        start_x += kSpacing;
    }

    return ret;
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
    if (id_ != 0) glDeleteProgram(id_);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    id_ = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
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
