# Vizualize and play your .wav files 
> [!NOTE]
> This project used to be a simple and small recorder device for another project of mine. It ended up becoming this mess.
## Demo
**Just drag & drop**
<div align="center">
<video src="https://github.com/user-attachments/assets/c7890901-9072-430c-94c9-ab943e43a1a8"></video><br>
</div>

It took *time* to get something visually appealing although it still looks weird, sometimes it's mesmerizing

# Compilation
## Windows using clang, Mac, Linux:
    make
## Windows MSVC:
    .\build.bat
May god help you compile it

# Dependencies
> [!WARNING]
> SDL3 with specific commit version must be installed if you don't want to rewrite macros and function names (they often change) as well as the corresponding version of SDL_TTF.
You should be able to compile it with latest versions; provided you have time to spend on SDL's documentation and commit messages..

- ![fftw](https://www.fftw.org/) binaries for an optimized version of the fast fourier transform is required (you might have 60 fps with a naive implementation tho)
- ![SDL3](https://github.com/libsdl-org/SDL/tree/6f199eabb) commit 6f199eabb
- ![SDL3_TTF](https://github.com/libsdl-org/SDL_ttf/tree/ad1ecab) commit ad1ecab
- ![SDL3_Image](https://github.com/libsdl-org/SDL_Image/tree/60013b1) commit 60013b1
