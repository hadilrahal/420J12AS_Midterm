#include <cstdlib>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

/*
 * EXAMEN DE MI-SESSION
 *
 * Consignes de remises) Créez un nouveau répertoire git et soumettez-le sur votre GitHub personnel.
 * Chaque question devrait être un COMMIT individuel, identifié par le numéro de la question. 10%
 *
 * Tous les outils traditionnels sont permis pour l'examen: recherche, forum de discussion, exemples
 * du cours, etc. L'IA est permis, mais just Copilot: https://copilot.microsoft.com/ et les IA
 * intégrées dans les IDEs. Pour copilot, s.v.p. ajoutez en commentaire le ou les lien(s) de votre
 * ou vos conversation(s) reliée(s) à l'examen (bouton en haut à droite). Vous pouvez être accusé de
 * plagiat autrement. De plus, il est interdit de demander verbatim les questions de l'examen dans
 * les conversations de copilot.
 *
 * Finalement, il n'est pas permis d'échanger des informations entre les étudiants (tolérance zéro)
 * ou de converser avec d'autres individus durant l'examen.
 *
 * - - - - -
 *
 * Question 1)	Modifiez les classes GameApp et Entity de sorte à supporter le patron du Composant
 * et du UpdateMethod. 20%
 *
 * Question 2) Mettez les composants nécessaires en place afin de pouvoir dessiner les entités sous
 * formes de rectangles de diverse grandeur, position et couleur. De plus, il faudra pouvoir
 * dessiner une Sprite pour certaines entités (les runes, voir ci-bas). La texture pour cela est
 * déjà chargée pour vous dans la classe GameApp. 40%
 *
 * Question 3) Créez les entités suivantes:
 * - Les runes: Les runes sont 3 entités pareilles, de taille 128x128, qui sont positionnées au
 * milieu de la fenêtre de jeu, à 1 quart, 2 quarts et 3 quarts de l'écran respectivement. Les runes
 * doivent être représenter par une SDL_Texture aléatoire du spritesheet 'runes.jpg'. Lorsqu'une
 * rune est cliquée, elle doit changer d'apparence au hasard.
 * - Le bouton: Le bouton est une entité de taille 220x40 qui apparaît à 50 pixels du bas de la
 * fenêtre. Lorsque le bouton est cliqué, les 3 runes doivent changer d'apparence au hasard. 30%
 * _____________________
 * |                   |
 * |    R    R    R    |
 * |                   |
 * |       [   ]       |
 * |-------------------|
 *
 */

static const std::string AppTitle = "Examen - Mi-session - [AJOUTER VOTRE NOM ET PRÉNOM ICI]";
static constexpr size_t MAX_SAMPLES = 100;

class GameApp final
{
	std::vector<float> _frame_times;

  public:
	SDL_Window *Window = nullptr;
	SDL_Renderer *Renderer = nullptr;
	SDL_Texture *Runes = nullptr;
	float SampleAverageFPS = 0.0f;

	GameApp ()
	{
		if (SDL_Init (SDL_INIT_VIDEO) == false)
			{
				SDL_LogCritical (1, "SDL failed to initialize! %s", SDL_GetError ());
				abort ();
			}
		Window = SDL_CreateWindow (AppTitle.c_str (), 800, 600, 0);
		if (Window == nullptr)
			{
				SDL_LogCritical (1, "SDL failed to create window! %s", SDL_GetError ());
				abort ();
			}
		Renderer = SDL_CreateRenderer (Window, nullptr);
		if (Renderer == nullptr)
			{
				SDL_LogCritical (1, "SDL failed to create renderer! %s", SDL_GetError ());
				abort ();
			}

		SDL_SetRenderVSync (Renderer, true);

		Runes = IMG_LoadTexture (Renderer, "res/runes.png");
		if (Runes == nullptr)
			{
				SDL_LogWarn (1, "SDL failed to load images! %s", SDL_GetError ());
			}
		SDL_SetTextureScaleMode (Runes, SDL_SCALEMODE_NEAREST);

		SDL_Time time;
		SDL_GetCurrentTime (&time);
		SDL_srand (time);
	}

	~GameApp ()
	{
		SDL_DestroyRenderer (Renderer);
		SDL_DestroyWindow (Window);
		SDL_Quit ();
	}

	void
	CalculateFPS (const float DeltaTime)
	{
		_frame_times.push_back (DeltaTime);
		if (_frame_times.size () > MAX_SAMPLES)
			{
				_frame_times.erase (_frame_times.begin ());
			}
		const float sum = std::accumulate (_frame_times.begin (), _frame_times.end (), 0.0f);
		const float average = sum / static_cast<float> (_frame_times.size ());
		SampleAverageFPS = average > 0 ? 1.0f / average : 0;
	}
};

Sint32
main (int argc, char *argv[])
{
	const auto app = new GameApp ();
	bool running = true;
	uint64_t last_time = SDL_GetPerformanceCounter ();
	while (running == true)
		{
			SDL_Event event;
			while (SDL_PollEvent (&event) == true)
				{
					if (event.type == SDL_EVENT_QUIT)
						{
							running = false;
						}
				}
			const uint64_t freq = SDL_GetPerformanceFrequency ();
			const uint64_t current_time = SDL_GetPerformanceCounter ();
			const float delta_time
				= static_cast<float> (current_time - last_time) / static_cast<float> (freq);
			last_time = current_time;
			app->CalculateFPS (delta_time);

			SDL_SetRenderDrawColor (app->Renderer, 12, 12, 44, 255);
			SDL_RenderClear (app->Renderer);

			SDL_Point win_size = { 0 };
			SDL_GetWindowSize (app->Window, &win_size.x, &win_size.y);

			SDL_SetRenderDrawColor (app->Renderer, 42, 42, 104, 255);
			SDL_RenderLine (app->Renderer, 0.f, 0.f, static_cast<float> (win_size.x),
							static_cast<float> (win_size.y));
			SDL_RenderLine (app->Renderer, 0.f, static_cast<float> (win_size.y),
							static_cast<float> (win_size.x), 0.f);

			static float displayed;
			static float count;
			if (count <= 0.f)
				{
					displayed = app->SampleAverageFPS;
					count = 90.f;
				}
			else
				{
					count -= delta_time * 1000.f;
				}
			SDL_SetRenderDrawColor (app->Renderer, 255, 255, 255, 255);
			SDL_RenderDebugTextFormat (app->Renderer, 5, 5, "%.2f FPS", displayed);

			SDL_RenderPresent (app->Renderer);
		}
	delete app;
	return 0;
}
