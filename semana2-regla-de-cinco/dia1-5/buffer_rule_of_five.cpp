// =====================================================================
// Semana 2: const-correctness + Regla de 3/5/0 + copia profunda + move
// =====================================================================
//
// Este archivo retoma el RaiiBuffer de la semana 1 (que solo permitía
// mover, no copiar) y lo evoluciona a una clase que SÍ puede copiarse
// (con copia profunda del recurso) y SÍ puede moverse (transfiriendo
// ownership sin duplicar memoria). El objetivo es entender, con datos
// reales impresos en consola, la diferencia entre ambas operaciones.

#include <iostream>
#include <cstddef>   // std::size_t
#include <string>    // std::string
#include <utility>   // std::move, std::forward, std::exchange
#include <algorithm> // std::copy
#include <stdexcept>

// ---------------------------------------------------------------------
// Día 2 (teoría): Regla de 3 / 5 / 0
// ---------------------------------------------------------------------
// - Regla de 3: si una clase gestiona un recurso manualmente (heap,
//   file handle, etc.) y por eso necesita definir un destructor
//   personalizado, entonces CASI SIEMPRE también necesita definir el
//   constructor de copia y el operador de copia. Si no los defines,
//   el compilador genera unos por defecto que copian el puntero
//   miembro a miembro (copia superficial / "shallow copy"), y dos
//   objetos terminan apuntando al mismo bloque de memoria -> doble
//   `delete` y comportamiento indefinido.
// - Regla de 5: en C++11+ se añaden el constructor de movimiento y el
//   operador de movimiento a la lista anterior. Si gestionas un
//   recurso, normalmente quieres definir los 5 (destructor, copia x2,
//   movimiento x2) o ninguno.
// - Regla de 0: la mejor RAII es la que no escribes tú: si delegas el
//   recurso en un miembro que ya gestiona su propio ciclo de vida
//   (std::vector, std::unique_ptr, std::string...), no necesitas
//   escribir ninguno de los 5 miembros especiales; el compilador los
//   genera correctamente porque cada miembro ya sabe copiarse/moverse
//   a sí mismo. Esta clase existe con fines didácticos (para practicar
//   la Regla de 5 a mano); en código de producción, preferiríamos la
//   Regla de 0 usando std::unique_ptr<int[]>.

class Buffer {
public:
    // --- Constructor: adquisición del recurso (RAII) ---
    explicit Buffer(std::size_t size)
        : size_(size), data_(size_ > 0 ? new int[size_] : nullptr)
    {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = 0;
        }
        std::cout << "[Buffer] Constructor: " << size_ << " ints en "
                  << static_cast<void*>(data_) << "\n";
    }

    // --- Destructor: liberación del recurso ---
    ~Buffer() {
        if (data_ != nullptr) {
            std::cout << "[Buffer] Destructor: liberando " << size_
                      << " ints en " << static_cast<void*>(data_) << "\n";
        } else {
            std::cout << "[Buffer] Destructor: nada que liberar "
                         "(vacío o movido)\n";
        }
        delete[] data_;
    }

    // -----------------------------------------------------------------
    // Día 2 (práctica): constructor y operador de COPIA -> copia
    // PROFUNDA. Cada objeto termina con su propio bloque de memoria
    // independiente, del mismo tamaño y contenido, pero en una
    // dirección distinta.
    // -----------------------------------------------------------------
    Buffer(const Buffer& other)
        : size_(other.size_),
          data_(other.size_ > 0 ? new int[other.size_] : nullptr)
    {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "[Buffer] Copy constructor: copia profunda de "
                  << static_cast<const void*>(other.data_) << " -> "
                  << static_cast<void*>(data_) << "\n";
    }

    Buffer& operator=(const Buffer& other) {
        if (this == &other) {
            return *this; // autoasignación: no hacer nada
        }

        // Reservamos primero el bloque nuevo; si `new` lanza, `this`
        // queda intacto (fuerte garantía de excepción).
        int* nuevo_bloque = other.size_ > 0 ? new int[other.size_] : nullptr;
        std::copy(other.data_, other.data_ + other.size_, nuevo_bloque);

        delete[] data_; // liberamos el recurso viejo
        data_ = nuevo_bloque;
        size_ = other.size_;

        std::cout << "[Buffer] Copy assignment: copia profunda hacia "
                  << static_cast<void*>(data_) << "\n";
        return *this;
    }

    // -----------------------------------------------------------------
    // Día 3 (práctica): constructor y operador de MOVIMIENTO -> se
    // transfiere el puntero (robo de recurso), sin duplicar memoria.
    // El objeto origen queda vacío (data_ == nullptr) y por eso su
    // destructor no libera nada.
    // -----------------------------------------------------------------
    Buffer(Buffer&& other) noexcept
        : size_(other.size_),
          data_(std::exchange(other.data_, nullptr))
    {
        other.size_ = 0;
        std::cout << "[Buffer] Move constructor: ownership transferido "
                     "(sin nueva reserva)\n";
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = other.size_;
            other.size_ = 0;
            std::cout << "[Buffer] Move assignment: ownership "
                         "transferido (sin nueva reserva)\n";
        }
        return *this;
    }

    // -----------------------------------------------------------------
    // Día 1: const-correctness.
    // - `size()` no modifica el objeto -> se marca `const`.
    // - `at()` tiene dos overloads: uno para objetos mutables (permite
    //   escribir) y uno para objetos const (solo lectura). El
    //   compilador elige automáticamente según la constancia del
    //   objeto que llama.
    // - `direccion()` y `imprimir()` solo leen estado -> también
    //   `const`, y reciben/devuelven referencias donde no hace falta
    //   copiar.
    // -----------------------------------------------------------------
    int& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Índice fuera de rango en Buffer::at");
        }
        return data_[index];
    }

    const int& at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Índice fuera de rango en Buffer::at");
        }
        return data_[index];
    }

    std::size_t size() const { return size_; }

    const void* direccion() const { return data_; }

    void imprimir(const std::string& etiqueta) const {
        std::cout << "  " << etiqueta << ": size=" << size_
                  << " data=" << direccion();
        if (size_ > 0) {
            std::cout << " [0]=" << data_[0];
        }
        std::cout << "\n";
    }

private:
    std::size_t size_;
    int* data_;
};

// ---------------------------------------------------------------------
// Día 3 (teoría): std::move y std::forward
// ---------------------------------------------------------------------
// - std::move NO mueve nada por sí mismo: es solo un cast a rvalue
//   reference (T&&). Le dice al compilador "trata este objeto como
//   temporal", habilitando que se elija el constructor/operador de
//   movimiento en vez del de copia. Por eso `other.data_` en el
//   constructor de movimiento de arriba se "roba" con std::exchange
//   (equivalente a mover manualmente un puntero).
// - std::forward se usa en plantillas con "forwarding references"
//   (T&&) para preservar la categoría de valor (lvalue o rvalue) del
//   argumento original al reenviarlo a otra función. Si en la función
//   de abajo usáramos `valor` directamente (sin forward), siempre se
//   trataría como lvalue dentro de la función y perderíamos la
//   posibilidad de mover el argumento.
void procesar(const std::string& s) {
    std::cout << "  procesar(const std::string&) -> copia/lectura: \"" << s << "\"\n";
}

void procesar(std::string&& s) {
    std::cout << "  procesar(std::string&&) -> se pudo mover: \"" << s << "\"\n";
}

template <typename T>
void reenviar_y_mostrar(T&& valor) {
    // std::forward<T> preserva si `valor` era lvalue o rvalue en el
    // punto de la llamada original.
    procesar(std::forward<T>(valor));
}

// ---------------------------------------------------------------------
// Día 4: validación manual de copia vs movimiento
// ---------------------------------------------------------------------
void demo_copia_duplica_recurso() {
    std::cout << "\n--- Día 4: copiar DUPLICA el recurso ---\n";
    Buffer original(3);
    original.at(0) = 111;

    Buffer copia(original); // copy constructor

    std::cout << "Direcciones antes de modificar la copia:\n";
    original.imprimir("original");
    copia.imprimir("copia");

    copia.at(0) = 999; // modificar la copia no debe afectar al original

    std::cout << "Después de modificar copia.at(0) = 999:\n";
    original.imprimir("original");
    copia.imprimir("copia");

    if (original.direccion() != copia.direccion() && original.at(0) == 111) {
        std::cout << "OK: direcciones distintas y 'original' no cambió "
                     "-> la copia es independiente (deep copy confirmada).\n";
    } else {
        std::cout << "FALLO: la copia comparte memoria con el original.\n";
    }
}

void demo_mover_no_duplica() {
    std::cout << "\n--- Día 4: mover TRANSFIERE sin duplicar ---\n";
    Buffer origen(3);
    origen.at(0) = 222;
    const void* direccion_original = origen.direccion();

    Buffer destino(std::move(origen)); // move constructor

    std::cout << "Después del move:\n";
    origen.imprimir("origen (ahora vacío)");
    destino.imprimir("destino");

    if (destino.direccion() == direccion_original && origen.direccion() == nullptr) {
        std::cout << "OK: 'destino' reutiliza la MISMA dirección que tenía "
                     "'origen', y 'origen' quedó vacío -> no hubo nueva "
                     "reserva ni copia de datos.\n";
    } else {
        std::cout << "FALLO: el move no transfirió el puntero como se esperaba.\n";
    }
}

// ---------------------------------------------------------------------
// Día 1: demostración de const-correctness
// ---------------------------------------------------------------------
void demo_const_correctness() {
    std::cout << "\n--- Día 1: const-correctness ---\n";
    const Buffer solo_lectura(2);
    // solo_lectura.at(0) = 5; // <-- no compila: at() const devuelve
    //                            const int&, no se puede asignar.
    std::cout << "  Leer un Buffer const: solo_lectura.at(0) = "
              << solo_lectura.at(0) << " (tamaño=" << solo_lectura.size()
              << ")\n";
    std::cout << "  Compila porque size()/at() const y direccion() están "
                 "marcados const: pueden invocarse sobre un objeto const.\n";
}

void demo_forwarding() {
    std::cout << "\n--- Día 3: std::move / std::forward ---\n";
    std::string texto = "hola";
    reenviar_y_mostrar(texto);              // lvalue -> procesar(const&)
    reenviar_y_mostrar(std::string("adios")); // rvalue -> procesar(&&)
}

int main() {
    demo_const_correctness();
    demo_forwarding();
    demo_copia_duplica_recurso();
    demo_mover_no_duplica();
    std::cout << "\nPrograma terminado sin fugas de memoria.\n";
    return 0;
}
