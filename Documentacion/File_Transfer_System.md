# Sistema de Transferencia de Archivos IRC

## Descripción General

Sistema simple pero funcional para transferir archivos entre clientes conectados a tu servidor IRC. Utiliza comandos IRC estándar extendidos (SEND, ACCEPT, REFUSE) y transmite archivos en chunks de 512 bytes.

## Arquitectura

### 1. **Clase File (File.hpp / file.cpp)**
Maneja la lectura y envío de archivos:

```cpp
class File {
    std::string name;        // Nombre del archivo
    std::string path;        // Ruta en el sistema
    std::string sender;      // Nick de quien envía
    std::string target;      // Nick del receptor
    
    unsigned long file_size; // Tamaño total
    unsigned long bytes_sent; // Bytes ya enviados
    
    bool open_file();           // Abre el archivo para lectura
    bool close_file();          // Cierra el archivo
    std::string read_chunk();   // Lee 512 bytes
    bool is_complete() const;   // Verifica si terminó
};
```

**Métodos principales:**
- `open_file()`: Abre el archivo y obtiene su tamaño
- `read_chunk()`: Lee los siguientes 512 bytes
- `is_complete()`: Verifica si se completó la transferencia
- `get_file_size()`: Retorna el tamaño total del archivo

### 2. **Comandos IRC**

#### **SEND <target_nick> <filepath>**
Inicia una transferencia de archivo.

**Ejemplo:**
```
SEND alice /path/to/document.pdf
```

**Proceso:**
1. Valida que el archivo existe
2. Busca al usuario destino
3. Abre el archivo y obtiene su tamaño
4. Notifica al receptor del inicio de la transferencia
5. Envía el archivo en chunks de 512 bytes
6. Notifica al receptor cuando termina

**Respuestas:**
```
461 SEND :Not enough parameters        # Faltan argumentos
401 <nick> :No such nick                # Usuario no existe
File not found: ...                     # Archivo no existe
Cannot open file: ...                   # No hay permisos de lectura
```

#### **ACCEPT <sender_nick> [save_path]**
Acepta una transferencia y especifica dónde guardar el archivo.

**Ejemplo:**
```
ACCEPT alice                    # Guarda con nombre por defecto
ACCEPT alice /tmp/document.pdf  # Guarda en ruta específica
```

#### **REFUSE <sender_nick>**
Rechaza una transferencia de archivo.

**Ejemplo:**
```
REFUSE alice
```

### 3. **Formato de Transmisión**

Durante la transferencia, se envían mensajes en formato:

```
:sender FT filepath sequence_number :contenido_en_base64_o_binario

Ejemplo:
:alice FT document.pdf 0 :JVBERi0xLjQKJeLj...
:alice FT document.pdf 1 :z0FBVCBYIFJlZiAx...
```

**Campos:**
- `alice`: Nick de quien envía
- `document.pdf`: Nombre del archivo
- `0`, `1`...: Número secuencial del chunk
- `:contenido`: Los datos del chunk (512 bytes máximo)

### 4. **Notificaciones al Usuario**

El sistema envía NOTICEs para informar del progreso:

```
# Al iniciador
NOTICE alice :Transferring document.pdf to bob

# Al receptor
NOTICE bob :Incoming file from alice: document.pdf (1048576 bytes)
NOTICE bob :File transfer complete from alice - document.pdf (1048576 bytes)
```

## Uso en Cliente IRC

### Cliente envía archivo:
```
PASS servidor_password
NICK alice
USER alice host.com server :Alice User
SEND bob /home/alice/document.pdf
```

### Cliente recibe archivo:
```
PASS servidor_password
NICK bob
USER bob host.com server :Bob User
ACCEPT alice /tmp/document.pdf
```

## Flujo Completo

```
[Alice]                          [Servidor]                    [Bob]
  |                                  |                          |
  |--- SEND bob file.pdf ----------->|                          |
  |                                  |--- Notify inicio ------->|
  |                                  |                          |
  |                                  |<--- ACCEPT alice --------|
  |                                  |                          |
  |                                  |--- FT chunk0 ----------->|
  |                                  |--- FT chunk1 ----------->|
  |                                  |--- FT chunk2 ----------->|
  |                                  |                          |
  |                                  |--- Notify fin ---------->|
  |<---- Notify completa ------------|                          |
```

## Mejoras Futuras

### Versión 1.1: Sistema de Confirmación
- Agregar ACKs para confirmar recepción de chunks
- Reintentos automáticos si falla un chunk
- Control de flujo (no enviar más de N chunks sin ACK)

### Versión 1.2: Almacenamiento Persistente
- Base de datos de transferencias en progreso
- Reanudación de transferencias interrumpidas
- Historial de transferencias

### Versión 2.0: Transferencias P2P Directas
- Conexión directa entre clientes (sin pasar por servidor)
- Mejor rendimiento para archivos grandes
- UDP para transferencias más rápidas

### Características Avanzadas
- Compresión de datos antes de transmitir
- Encriptación de archivos
- Transferencia múltiple simultánea
- Límite de velocidad configurable
- Vista previa de archivos (imágenes pequeñas)

## Integración con Makefile

Asegúrate de compilar los nuevos archivos:

```makefile
SRCS = 	srcs/Messaging/commands.cpp \
		srcs/Messaging/router.cpp \
		srcs/Messaging/file.cpp \
		...
```

## Ejemplo de Implementación Completa

```cpp
// Enviar archivo de 2MB en chunks de 512 bytes
SEND bob large_file.bin

// El cliente recibe notificaciones:
// NOTICE bob :Incoming file from alice: large_file.bin (2097152 bytes)
// Luego recibe 4096 mensajes FT (2097152 / 512 = 4096 chunks)
// Finalmente: NOTICE bob :File transfer complete...

// El cliente puede procesar cada chunk así:
if (message.contains("FT chunk"))
{
    unsigned char chunk[512];
    std::istringstream iss(message);
    iss >> header >> sender >> "FT" >> filename >> seq;
    // Leer bytes y guardar a archivo
    file.write(chunk, bytes_read);
}
```

## Limitaciones Actuales

1. **Sin compresión**: Se transmite tamaño completo
2. **Sin confirmación**: Si falla un chunk, no se detecta
3. **Sin reanudación**: Si se interrumpe, hay que empezar de novo
4. **Síncrono**: Bloquea el servidor mientras envía
5. **IRC estándar**: Tamaño máximo de mensaje ~512 bytes

## Soluciones a Limitaciones

Para servidor más robusto:
1. **Hacer asíncrono**: Encolar apenas algunos chunks, no todos
2. **Implementar ACKs**: Receptor confirma cada chunk recibido
3. **Hash de integridad**: SHA256 del archivo al completar
4. **Caché de transferencias**: Guardar estado en Server

---

**Autor**: Implementación basada en arquitectura IRC existente  
**Fecha**: 2024  
**Versión del Sistema**: 1.0
