
#include <iostream>
#include <cstddef>   // std::size_t
#include <utility>   // std::move, std::exchange
#include <stdexcept>

class RaiiBuffer {
public:
    // --- Constructor: aquí ocurre la "A" de RAII (Adquisición) ---
    explicit RaiiBuffer(std::size_t size)
        : size_(size), data_(new int[size]) // pedimos memoria en el heap
    {
        std::cout << "[RaiiBuffer] Constructor: asignando "
                  << size_ << " ints (" << size_ * sizeof(int)
                  << " bytes) en " << data_ << "\n";

        // Inicializamos el buffer en 0, solo para tener datos "limpios"
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = 0;
        }
    }

    // --- Destructor: aquí ocurre la "L" de RAII (Liberación) ---
    // Este destructor se llama SIEMPRE que el objeto sale de scope,
    // sin que nosotros tengamos que acordarnos de llamar delete[].
    ~RaiiBuffer() {
        if (data_ != nullptr) {
            std::cout << "[RaiiBuffer] Destructor: liberando "
                      << size_ << " ints en " << data_ << "\n";
        } else {
            std::cout << "[RaiiBuffer] Destructor: nada que liberar "
                      << "(el buffer fue movido a otro objeto)\n";
        }
        delete[] data_;
    }

    // --- Prohibimos COPIAR (duplicar ownership) ---
    RaiiBuffer(const RaiiBuffer&) = delete;
    RaiiBuffer& operator=(const RaiiBuffer&) = delete;

    // --- Permitimos MOVER (transferencia de ownership) ---
    // Mover significa: "el nuevo objeto se queda con el puntero,
    // y el objeto viejo se queda en nullptr (vacío, sin nada que liberar)".
    // Esto es exactamente lo mismo que hace unique_ptr internamente.
    RaiiBuffer(RaiiBuffer&& other) noexcept
        : size_(other.size_),
          data_(std::exchange(other.data_, nullptr)) // robamos el puntero
    {
        other.size_ = 0;
        std::cout << "[RaiiBuffer] Move constructor: ownership transferido\n";
    }

    RaiiBuffer& operator=(RaiiBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;                     // liberamos lo que ya teníamos
            data_ = std::exchange(other.data_, nullptr);
            size_ = other.size_;
            other.size_ = 0;
            std::cout << "[RaiiBuffer] Move assignment: ownership transferido\n";
        }
        return *this;
    }

    // --- Acceso seguro a los datos ---
    int& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Índice fuera de rango en RaiiBuffer::at");
        }
        return data_[index];
    }

    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};

// ---------------------------------------------------------
// Programa de prueba: aquí validamos "a mano" con prints que
// todo se asigna y libera correctamente, en el orden esperado.
// ---------------------------------------------------------
void ejemplo_basico() {
    std::cout << "\n--- ejemplo_basico() ---\n";
    RaiiBuffer buffer(5);
    buffer.at(0) = 42;
    std::cout << "buffer.at(0) = " << buffer.at(0) << "\n";
    // Al salir de esta función, el destructor libera el buffer solo.
}

void ejemplo_move() {
    std::cout << "\n--- ejemplo_move() ---\n";
    RaiiBuffer original(3);
    original.at(0) = 100;

    std::cout << "Moviendo 'original' a 'nuevo'...\n";
    RaiiBuffer nuevo(std::move(original));
    // 'original' ahora está vacío (data_ == nullptr), 'nuevo' es el dueño real.

    std::cout << "nuevo.at(0) = " << nuevo.at(0) << "\n";
    // Al terminar la función: destructor de 'original' (no hace nada)
    // y destructor de 'nuevo' (libera la memoria real).
}

void ejemplo_excepcion() {
    std::cout << "\n--- ejemplo_excepcion() ---\n";
    try {
        RaiiBuffer buffer(2);
        buffer.at(10) = 1; // esto lanza std::out_of_range
    } catch (const std::out_of_range& e) {
        std::cout << "Excepción capturada: " << e.what() << "\n";
        std::cout << "(Aun así, el destructor de 'buffer' ya se ejecutó "
                     "antes de llegar aquí -> sin fugas)\n";
    }
}

int main() {
    ejemplo_basico();
    ejemplo_move();
    ejemplo_excepcion();
    std::cout << "\nPrograma terminado sin fugas de memoria.\n";
    return 0;
}