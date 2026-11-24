# Documentación del trabajo final de psinfo en xv6

Esta carpeta reúne el informe, la presentación y el parche del proyecto de **Sistemas Operativos (IS-380)** de la Universidad Nacional de San Cristóbal de Huamanga. El proyecto incorpora `psinfo` a xv6-riscv para consultar desde el shell el PID, el PPID, el estado, el tamaño virtual y el nombre de los procesos.

## Archivos

| Archivo | Contenido |
| --- | --- |
| [Informe final en PDF](Informe_final_psinfo_xv6.pdf) | Versión de lectura y entrega. Presenta el problema, el diseño, la implementación y la evaluación. |
| [Informe final en Word](Informe_final_psinfo_xv6.docx) | Versión editable del informe. |
| [Presentación en PowerPoint](Exposicion_psinfo_xv6.pptx) | Material para exponer el proyecto y explicar la demostración. |
| [Documentación complementaria](documentacion-complementaria/) | Carpeta que contiene el [parche unificado de la implementación](documentacion-complementaria/parches.patch). |

## Compilación y pruebas

Consulta el [README principal del repositorio](../README.md) para compilar xv6 y ejecutar `psinfo` y `psinfotest` en QEMU. Para reproducir la implementación desde una copia limpia de xv6-riscv, usa el parche y sigue la revisión base y los pasos de reproducción indicados en el informe.
