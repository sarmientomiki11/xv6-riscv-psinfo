# xv6-riscv con psinfo para el monitoreo de procesos

Trabajo final de **Sistemas Operativos (IS-380)**, Escuela Profesional de Ingeniería de Sistemas, Universidad Nacional de San Cristóbal de Huamanga (UNSCH), 2026. Docente **Lizbeth Jaico Quispe**.

## Integrantes

| Integrante | Cuenta de GitHub |
| --- | --- |
| Avila Alanya Neftaly Dianel | [Neftaly-Avila](https://github.com/Neftaly-Avila) |
| Balboa Yupanqui, Pol Deywi | [polbalboa27-hash](https://github.com/polbalboa27-hash) |
| Sarmiento Medina, Mijael | [sarmientomiki11](https://github.com/sarmientomiki11) |

## Descripción

Este proyecto incorpora `psinfo`, un comando que permite consultar los procesos de xv6 desde su shell. Se basa en [xv6-riscv de MIT](https://github.com/mit-pdos/xv6-riscv/tree/9e3161a9abf5f51ea402562d1874caf6c4926597), revisión `9e3161a9abf5f51ea402562d1874caf6c4926597`.

- El comando muestra el PID, el PID del padre (PPID), el estado, el tamaño virtual en bytes y el nombre del proceso.
- La llamada al sistema `getpsinfo` valida la capacidad y el búfer de destino y entrega las filas al programa mediante `copyout`.
- El núcleo obtiene los datos de cada fila bajo bloqueo. El listado incluye procesos zombis hasta que se recolectan y omite entradas libres o en inicialización.
- El programa `psinfotest` ejecuta ocho pruebas automáticas de la interfaz.

## Requisitos

En VS Code, abre la carpeta raíz del proyecto, donde están `Makefile`, `kernel/` y `user/`. Realiza la compilación y las pruebas desde la terminal de una máquina virtual Linux con una copia de los archivos del proyecto. Se requiere QEMU 7.2 o superior y un compilador cruzado RISC-V de 64 bits.

### Ubuntu y Debian

```bash
sudo apt update
sudo apt install git build-essential perl bc qemu-system-misc \
  gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

```bash
qemu-system-riscv64 --version
riscv64-linux-gnu-gcc --version
```

## Compilación y ejecución

Desde la carpeta raíz de la copia del proyecto que ya contiene la extensión, compila el núcleo y crea la imagen del sistema de archivos. La salida de `mkfs` debe incluir `_psinfo` y `_psinfotest`.

```bash
make TOOLPREFIX=riscv64-linux-gnu- kernel/kernel fs.img
make TOOLPREFIX=riscv64-linux-gnu- CPUS=1 qemu
```

Cuando aparezca el prompt `$`, ejecuta los comandos de la sección [Uso](#uso). Para salir de QEMU, presiona **Ctrl+A** y luego **X**. Repite la prueba con cuatro CPU virtuales.

```bash
make TOOLPREFIX=riscv64-linux-gnu- CPUS=4 qemu
```

Para limpiar los archivos de compilación, ejecuta `make clean` en la terminal Linux de la máquina virtual.

## Uso

### Llamada al sistema

| N.º | Prototipo | Resultado |
| --- | --- | --- |
| 28 | `int getpsinfo(struct psinfo *rows, int cap);` | Devuelve el número de filas copiadas o `-1` si falla. La capacidad debe estar entre 1 y `NPROC`. |

La estructura compartida `struct psinfo` ocupa 40 bytes por fila. `VIRT_B` mide el tamaño virtual del proceso según `p->sz` y puede incluir páginas aún sin respaldo físico. Los estados exportados son `SLEEPING`, `RUNNABLE`, `RUNNING` y `ZOMBIE`.

### Comandos dentro de xv6

| Comando | Qué hace |
| --- | --- |
| `psinfo` | Muestra las columnas `PID`, `PPID`, `ESTADO`, `VIRT_B` y `NOMBRE`. |
| `psinfo extra` | Muestra el mensaje de uso y termina con código 2. |
| `psinfotest` | Ejecuta las ocho pruebas y muestra el total aprobado. |

```text
ls
psinfo
psinfo extra
psinfotest
```

En el listado de `psinfo` deben aparecer las filas de `init`, `sh` y `psinfo`. Los PID y los tamaños observados dependen de la ejecución.

## Pruebas

Ejecuta `psinfotest` tres veces con `CPUS=1` y tres veces con `CPUS=4`. Si pasan todas las pruebas, cada corrida termina con `TOTAL 8/8 PASS`.

| Caso | Qué verifica | Resultado esperado |
| --- | --- | --- |
| T01 | Integridad del listado | Las filas tienen valores válidos, los PID son únicos y aparece el propio proceso. |
| T02 | Capacidad del búfer | Con capacidad 1 se escribe una sola fila y se rechazan las capacidades fuera de rango. |
| T03 | Validación del destino | Los destinos inválidos devuelven `-1` y una consulta válida posterior funciona. |
| T04 | Hijo bloqueado | El hijo aparece en `SLEEPING` con el PPID esperado. |
| T05 | Estado zombi y recolección | El hijo aparece en `ZOMBIE` y deja de aparecer después de `wait`. |
| T06 | Cambio de tamaño virtual | El tamaño aumenta 8192 bytes y recupera el valor inicial al reducirse. |
| T07 | Consultas concurrentes | El padre y los hijos realizan consultas y los tres hijos terminan correctamente. |
| T08 | Copia entre páginas | Se copia correctamente una fila que cruza el límite de una página de usuario. |

Para el informe, conserva la salida de la compilación que genera `fs.img`, el listado de `psinfo` y los resultados de las pruebas con 1 y 4 CPU. Los recuadros del Apéndice D indican qué debe mostrar cada captura.

## Archivos modificados o creados

| Archivo | Cambio |
| --- | --- |
| `kernel/psinfo.h` | Nuevo. Define el formato y los estados de las filas. |
| `kernel/proc.c` | Obtiene las filas de procesos y las copia al espacio de usuario. |
| `kernel/proc.h` | Documenta la protección del nombre y el tamaño virtual. |
| `kernel/exec.c` | Actualiza el nombre y el tamaño bajo bloqueo al ejecutar otro programa. |
| `kernel/defs.h` | Declara `getpsinfo` para el núcleo. |
| `kernel/sysproc.c` | Implementa el controlador de la llamada y protege el cambio de tamaño diferido. |
| `kernel/syscall.h` | Define el número 28. |
| `kernel/syscall.c` | Registra el controlador en la tabla de llamadas. |
| `user/user.h` | Declara el prototipo de usuario. |
| `user/usys.pl` | Genera la rutina de entrada de la llamada. |
| `user/psinfo.c` | Nuevo. Implementa el listado de procesos. |
| `user/psinfotest.c` | Nuevo. Implementa las pruebas T01 a T08. |
| `Makefile` | Incluye ambos programas en `fs.img`. |

## Reproducir desde un xv6 limpio

Guarda el parche unificado del Apéndice A del informe como `psinfo.patch` en el directorio desde el que vas a clonar xv6. El comando `git apply --check` comprueba que los cambios encajan en la base antes de aplicarlos.

```bash
git clone https://github.com/mit-pdos/xv6-riscv.git xv6-psinfo
cd xv6-psinfo
git switch -c psinfo 9e3161a9abf5f51ea402562d1874caf6c4926597
git apply --check ../psinfo.patch
git apply ../psinfo.patch
git diff --check
```

También puedes seguir en VS Code los pasos de la guía **Guia_codigos_psinfo_xv6_VS_Code.docx**, que contiene los tres archivos nuevos y los bloques que se reemplazan en los archivos existentes.

## Limitaciones

- El recorrido ofrece coherencia por fila. Los procesos pueden cambiar de estado entre la lectura de dos filas.
- `VIRT_B` indica el tamaño virtual. La interfaz se limita a los campos de identificación, estado, tamaño virtual y nombre.
- La extensión usa la política de planificación de la revisión base.

## Créditos y licencia

Basado en [xv6-riscv](https://github.com/mit-pdos/xv6-riscv) de Russ Cox, Frans Kaashoek y Robert Morris, MIT. El código base se distribuye bajo la licencia MIT. Consulta [LICENSE](LICENSE).
