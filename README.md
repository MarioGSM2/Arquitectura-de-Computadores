# Arquitectura-de-Computadores

Este repositorio ahora incluye un ejemplo completo de cómo evolucionar el código
original de geolocalización hacia una solución dinámica dividida en tres etapas.
Cada etapa incrementa las capacidades hasta llegar a una experiencia con
backend en tiempo real y un dashboard web interactivo.

## Requisitos

Instala las dependencias de Python (se recomienda un entorno virtual):

```bash
pip install -r requirements.txt
```

## ¿Cómo está organizado el código?

El repositorio se divide en dos bloques principales:

| Carpeta | Propósito | Ejemplos de archivos |
| --- | --- | --- |
| `app_core/` | Componentes reutilizables que comparten las tres etapas. Aquí viven la configuración, el manejo del polígono, el almacenamiento en memoria y el render del mapa. | `config.py`, `data_store.py`, `geo.py`, `map_renderer.py` |
| `stages/` | Implementaciones completas listas para ejecutarse. Cada subcarpeta es un paquete Python independiente con su propio `__init__.py` y un archivo principal. | `stage01_static_map/main.py`, `stage02_backend/app.py`, `stage03_dashboard/streamlit_app.py` |

Cuando ejecutes `python -m stages.stage01_static_map.main`, Python buscará el paquete `stages`, luego la subcarpeta `stage01_static_map` y finalmente el módulo `main.py`. Ese mismo patrón se usa para el backend (`stages.stage02_backend.app`) y el dashboard (`stages.stage03_dashboard.streamlit_app`).

### Trabajar con esta estructura en Google Colab

0. **Opcional pero recomendado: monta Google Drive para conservar los archivos.**
   ```python
   from google.colab import drive
   drive.mount("/content/drive")
   ```
   - En el panel web de Drive verás la ruta de tu carpeta en la parte superior.
     En la imagen de ejemplo, el repositorio se quiere guardar dentro de
     `Mi unidad ▸ inalP`, que en Colab se representa como la ruta
     `/content/drive/MyDrive/inalP` ("Mi unidad" siempre se traduce a
     `MyDrive`).
   - Si la carpeta `inalP` aún no existe, créala desde la interfaz de Drive
     o ejecuta `!mkdir -p /content/drive/MyDrive/inalP` antes de clonar.
   - Puedes escoger otra carpeta diferente; solo asegúrate de adaptar las
     rutas en los comandos siguientes.

1. **Clona el repositorio en la carpeta que prefieras.**
   - Si es la primera vez, sitúate en la ruta deseada (Drive o `/content`):
     ```python
     %cd /content/drive/MyDrive/inalP  # ajusta la ruta según tu carpeta en Drive
     ```
   - Clona usando HTTPS (sustituye la URL si trabajas sobre tu propio fork):
     ```python
     !git clone https://github.com/<tu-usuario>/Arquitectura-de-Computadores.git
     ```
     > 💡 Si el repositorio es privado o quieres subir cambios, utiliza un
     > [token de acceso personal](https://github.com/settings/tokens). Ejecuta
     > `git config user.name "Tu Nombre"` y `git config user.email "tu@email"`
     > para configurar tu identidad antes de hacer `git commit`.

2. **Colócate dentro del repositorio**. Tras clonar, ejecuta siempre:
   ```python
   %cd /content/drive/MyDrive/inalP/Arquitectura-de-Computadores  # ajusta la ruta según la carpeta donde clonaste
   ```
   Comprueba la ubicación con `!pwd` y enumera carpetas con `!ls`.

3. **Editar o crear archivos en `app_core/`**. Usa la celda mágica `%%writefile` para escribir el contenido completo del archivo desde Colab:
   ```python
   %%writefile app_core/nuevo_modulo.py
   """Ejemplo de utilidades compartidas."""

   def ejemplo():
       return "Hola desde app_core"
   ```
   Para revisar su contenido, ejecuta `!sed -n '1,160p' app_core/nuevo_modulo.py`.

4. **Editar los módulos de `stages/`**. El procedimiento es idéntico, solo cambia la ruta. Por ejemplo, para actualizar el `main.py` de la etapa 1:
   ```python
   %%writefile stages/stage01_static_map/main.py
   # contenido completo del script aquí
   ```
   Cada subcarpeta (`stage01_static_map`, `stage02_backend`, `stage03_dashboard`) ya contiene un `__init__.py` vacío que le indica a Python que es un paquete, por lo que no es necesario crearlo manualmente.

5. **Importaciones internas**. Desde cualquier archivo puedes reutilizar los módulos compartidos haciendo:
   ```python
   from app_core.geo import get_campus_geometry
   from app_core.data_store import InMemoryStore
   ```
   Asegúrate de haber añadido la ruta del repositorio a `sys.path` (ver la sección de Colab más abajo) antes de ejecutar los scripts.

6. **Guardar cambios**. Colab no tiene un editor gráfico para repositorios, pero cualquier celda que use `%%writefile` sobrescribe el archivo en disco. Puedes verificar el resultado con `!git status` y, si trabajas en tu propio fork, subir los cambios con los comandos `git` habituales. Si el repositorio vive en Drive, tus modificaciones persistirán entre sesiones de Colab.

Con esta guía sabrás exactamente dónde ubicar tus funciones reutilizables (`app_core/`) y dónde dejar los scripts listos para ejecutarse (`stages/`) mientras trabajas en Google Colab.

### ¿Cómo actualizo mi copia con los últimos cambios de GitHub?

Si el repositorio original recibe actualizaciones, puedes sincronizar tu copia de Colab (o de Drive) con los comandos estándar de Git:

1. **Comprueba tus remotos** (la URL desde la que clonaste y, opcionalmente, la del repositorio original):
   ```python
   !git remote -v
   ```
   - Si hiciste un fork, suele haber dos remotos: `origin` (tu fork) y `upstream` (el repositorio original). Si solo aparece `origin` y necesitas el original, agrégalo:
     ```python
     !git remote add upstream https://github.com/ORIGINAL/Arquitectura-de-Computadores.git
     ```

2. **Guarda o descarta tus cambios locales**. Git no permite hacer `pull` cuando hay archivos modificados sin commitear. Usa `!git status` para revisar y `!git commit -am "mensaje"` o `!git checkout -- ruta/del/archivo` según corresponda.

3. **Descarga las novedades**:
   - Para traer las actualizaciones del remoto desde el que clonaste (habitualmente tu fork):
     ```python
     !git pull origin main  # sustituye "main" por la rama que uses, por ejemplo "work"
     ```
   - Si necesitas sincronizarte con el repositorio original, primero descarga y luego fusiona:
     ```python
     !git fetch upstream
     !git merge upstream/main  # ajusta el nombre de la rama si es distinto
     ```

4. **Resuelve conflictos si aparecen**. Git mostrará qué archivos se solapan; edítalos en Colab con `%%writefile`, luego `!git add archivo` y finaliza con `!git commit`.

5. **(Opcional) Sube los cambios actualizados a tu fork**:
   ```python
   !git push origin work  # o la rama que estés utilizando
   ```

Con este flujo tu repositorio en Colab/Drive permanecerá alineado con los últimos cambios de GitHub antes de continuar trabajando.

## Etapa 1 – Script modular

Ubicación: [`stages/stage01_static_map`](stages/stage01_static_map).

1. Obtiene el polígono del campus.
2. Permite registrar puntos manuales en memoria.
3. Renderiza un mapa HTML con el contorno y los puntos dibujados.

Ejecución:

```bash
python -m stages.stage01_static_map.main
```

El archivo `stage1_map.html` quedará disponible en el directorio raíz.

## Etapa 2 – API en FastAPI

Ubicación: [`stages/stage02_backend`](stages/stage02_backend).

1. Expone un endpoint `POST /locations` para recibir datos en tiempo real.
2. Permite consultar el historial filtrando por fecha (`GET /locations`).
3. Entrega el mapa actualizado como HTML (`GET /map`).
4. Permite limpiar el historial (`POST /reset`).

Ejecución (recarga automática):

```bash
uvicorn stages.stage02_backend.app:app --reload
```

### Exponer la API desde Google Colab u otros entornos remotos

Para que `pyngrok` funcione necesitas **registrarte** y configurar tu
**authtoken** personal. Sin esto aparecerá el error `ERR_NGROK_4018`
(`authentication failed`). Sigue el paso a paso:

1. **Crear la cuenta y obtener el token**
   - Visita [dashboard.ngrok.com/signup](https://dashboard.ngrok.com/signup) y crea una cuenta gratuita.
   - Una vez dentro del panel, abre la sección **Your Authtoken**
     ([enlace directo](https://dashboard.ngrok.com/get-started/your-authtoken)) y copia el texto parecido a `2P9x...`.

2. **Instalar pyngrok en Colab** (si aún no lo tienes)

   ```python
   !pip install pyngrok
   ```

3. **Registrar el authtoken**
   - Ejecuta una sola vez por sesión (o guárdalo en Google Drive si quieres reutilizarlo):

     ```python
     from pyngrok import ngrok
     ngrok.set_auth_token("TU_AUTHTOKEN")  # pega aquí el token copiado
     ```

   - Este comando crea/actualiza el archivo de configuración de ngrok dentro del entorno de Colab. No imprime nada, pero si el token es incorrecto mostrará un error explicativo.

4. **Abrir el túnel**

   ```python
   public_url = ngrok.connect(8000, "http")
   print("Public URL:", public_url)
   ```

   - El valor mostrado (`http://...ngrok-free.app`) será la dirección pública que debes usar desde el dashboard o cualquier cliente externo.

5. **Verificar y cerrar túneles (opcional)**

   ```python
   ngrok.get_tunnels()     # lista los túneles activos
   ngrok.disconnect(public_url.public_url)  # cierra un túnel concreto
   ```

Si prefieres evitar el registro, puedes trabajar en local y acceder a
`http://localhost:8000`, o emplear alternativas como
[localtunnel](https://github.com/localtunnel/localtunnel) o los
túneles propios de tu plataforma (por ejemplo, `cloudflared` o `serveo`).

## Etapa 3 – Dashboard en Streamlit

Ubicación: [`stages/stage03_dashboard`](stages/stage03_dashboard).

1. Consume la API de la etapa 2 para obtener y mostrar datos.
2. Renderiza el mapa dentro de la interfaz web y una tabla filtrable.
3. Permite añadir puntos manualmente desde la UI.

Ejecución:

```bash
streamlit run stages/stage03_dashboard/streamlit_app.py
```

> **Nota:** la aplicación asume que la API se encuentra en
> `http://localhost:8000`. Puedes cambiar la variable de entorno
> `TRACKER_API_URL` para apuntar a otra URL.

## Flujo completo

### Opción A: en tu máquina local

1. **Terminal 1 – backend**
   ```bash
   uvicorn stages.stage02_backend.app:app --reload
   ```
   El servidor quedará escuchando en `http://localhost:8000`.

2. **Terminal 2 – dashboard**
   ```bash
   streamlit run stages/stage03_dashboard/streamlit_app.py
   ```
   Streamlit abrirá el navegador apuntando a `http://localhost:8501`.

3. **Flujo de datos**
   - Desde la UI puedes añadir ubicaciones manualmente.
   - También puedes enviar puntos desde otro proceso con `curl` o `requests`:
     ```bash
     curl -X POST http://localhost:8000/locations \
       -H "Content-Type: application/json" \
       -d '{"latitude": 3.3415, "longitude": -76.5295, "label": "Sensor A"}'
     ```

### Opción B: en Google Colab (una sola instancia)

1. **Arranca el backend en segundo plano**
   ```python
   # Celda de Colab
   !uvicorn stages.stage02_backend.app:app --host 0.0.0.0 --port 8000 --reload &
   ```
   La celda seguirá mostrando logs; deja el proceso activo.

   > 🔧 **¿`ModuleNotFoundError: No module named 'stages'`?**
   > Asegúrate de que **la celda anterior ejecutó `%cd Arquitectura-de-Computadores`**
   > y de que el directorio actual aparece en `sys.path`. En Colab puedes forzar
   > esto ejecutando una celda con:
   > ```python
   > import os, sys
   > repo_path = "/content/Arquitectura-de-Computadores"  # ajusta si tu ruta es distinta
   > os.chdir(repo_path)
   > sys.path.append(repo_path)
   > !pwd  # verifica que estás dentro del repositorio
   > ```
   > Después vuelve a lanzar el comando de `uvicorn`.

2. **Abre el túnel (si necesitas acceso externo)**
   ```python
   from pyngrok import ngrok
   ngrok.set_auth_token("TU_AUTHTOKEN")  # solo la primera vez
   public_api = ngrok.connect(8000, "http")
   public_api
   ```

3. **Arranca el dashboard en otro puerto**
   ```python
   !streamlit run stages/stage03_dashboard/streamlit_app.py \
       --server.port 8501 --server.headless true &
   ```
   Streamlit levanta una aplicación web independiente y por defecto usa el
   puerto **8501** (mientras que la API escucha en el **8000**). “Arrancarlo en
   otro puerto” solo significa que ambos servicios pueden convivir a la vez en
   la misma máquina porque cada uno atiende peticiones en un número de puerto
   diferente. Si necesitas usar un puerto distinto (por ejemplo, porque 8501 ya
   está ocupado), cambia `--server.port` por el número disponible que prefieras.

   > 📌 **Consejo:** si el comando parece quedarse “bloqueado”, es normal: el
   > proceso queda activo y emite logs en la celda. Para ejecutar nuevas celdas,
   > simplemente abre una celda debajo; no necesitas detener el dashboard a
   > menos que quieras reiniciarlo (usa `!pkill -f streamlit` para cerrarlo).

4. **Abre un túnel para el dashboard (opcional)**
   ```python
   public_dashboard = ngrok.connect(8501, "http")
   public_dashboard
   ```

5. **Sincroniza la URL de la API**
   - Si usas túnel, pasa la dirección al dashboard antes de abrirlo:
     ```python
     import os
     os.environ["TRACKER_API_URL"] = str(public_api)
     ```
     Ejecuta esta celda antes del comando de Streamlit.

6. **Interacción en Colab**
   - Visita la URL del dashboard para añadir puntos o consultarlos.
   - Desde Colab, puedes enviar datos con `requests`:
     ```python
     import requests
     requests.post("http://127.0.0.1:8000/locations", json={
         "latitude": 3.3420,
         "longitude": -76.5288,
         "label": "Lectura remota"
     })
     ```
   - Los puntos que agregues aparecerán en la tabla y en el mapa en segundos.

> 💡 En Colab los procesos se detienen al cerrar la pestaña o si la sesión se
> reinicia. Para conservar tus cambios, monta Google Drive y guarda allí tus
> archivos modificados.

## Estructura del código compartido

Los módulos en [`app_core`](app_core) contienen la lógica reutilizable entre
las tres etapas:

- `config.py`: constantes globales (ubicación, zoom, límites de historial).
- `geo.py`: descarga y cachea el polígono desde OpenStreetMap.
- `data_store.py`: almacena los puntos en memoria usando pandas.
- `map_renderer.py`: compone el mapa con folium.

Estas utilidades permiten evolucionar el prototipo sin duplicar código.
