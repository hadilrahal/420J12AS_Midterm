#include <cstdlib>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <algorithm>   // (ajout Q1)
#include "entity.h"    // (ajout Q1)

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

// ====================== (ajout Q2) Composants de rendu ======================
// (ajout Q2) je crée Transform (position/taille), RectRender (couleur), SpriteRender (texture + srcRect)

struct TransformComponent : public Component
{
	SDL_FRect Rect{ 0.f, 0.f, 10.f, 10.f };
	TransformComponent(float x, float y, float w, float h) { Rect = { x, y, w, h }; }
};

struct RectRenderComponent : public Component
{
	SDL_Color Color{ 255, 255, 255, 255 };
	explicit RectRenderComponent(SDL_Color c) : Color(c) {}

	void Draw(Entity& owner, SDL_Renderer* renderer) override
	{
		auto* t = owner.GetComponent<TransformComponent>();
		if (!t) return;

		SDL_SetRenderDrawColor(renderer, Color.r, Color.g, Color.b, Color.a);
		SDL_RenderFillRect(renderer, &t->Rect);
	}
};

struct SpriteRenderComponent : public Component
{
	SDL_Texture* Texture = nullptr;
	SDL_FRect Src{ 0.f, 0.f, 0.f, 0.f };

	SpriteRenderComponent(SDL_Texture* tex, SDL_FRect src)
		: Texture(tex), Src(src) {}

	void Draw(Entity& owner, SDL_Renderer* renderer) override
	{
		auto* t = owner.GetComponent<TransformComponent>();
		if (!t || !Texture) return;

		// (ajout Q2) je dessine la sous-image Src de la texture dans le rectangle destination
		SDL_RenderTexture(renderer, Texture, &Src, &t->Rect);
	}
};

// ====================== (ajout Q3) Input + Click helper ======================

struct MouseInput
{
	float x = 0.f;
	float y = 0.f;
	bool leftDown = false;
	bool leftPressedThisFrame = false;
	bool leftReleasedThisFrame = false;

	void BeginFrame()
	{
		leftPressedThisFrame = false;
		leftReleasedThisFrame = false;
	}
};

static bool PointInRect(float px, float py, const SDL_FRect& r)
{
	return (px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h);
}

// (ajout Q3) composant cliquable : si je clique dans le rect, j'appelle une action
struct ClickableComponent : public Component
{
	MouseInput* Mouse = nullptr;
	// je stocke une fonction à appeler quand c'est cliqué
	std::function<void(Entity&)> OnClick;

	ClickableComponent(MouseInput* m, std::function<void(Entity&)> cb)
		: Mouse(m), OnClick(std::move(cb)) {}

	void Update(Entity& owner, float) override
	{
		if (!Mouse || !OnClick) return;
		auto* t = owner.GetComponent<TransformComponent>();
		if (!t) return;

		if (Mouse->leftPressedThisFrame && PointInRect(Mouse->x, Mouse->y, t->Rect))
			OnClick(owner);
	}
};

// (ajout Q3) je retourne une case au hasard dans le spritesheet (128x128 par case)
static SDL_FRect RandomRuneSrc()
{
	// si ton image runes.png n'est pas une grille 4x4 de 128, ajuste COLS/ROWS.
	const float CELL_W = 128.f;
	const float CELL_H = 128.f;
	const int COLS = 4;
	const int ROWS = 4;

	const int total = COLS * ROWS;
	const int idx = SDL_rand(total); // 0..total-1
	const int cx = idx % COLS;
	const int cy = idx / COLS;

	return SDL_FRect{ cx * CELL_W, cy * CELL_H, CELL_W, CELL_H };
}

class GameApp final
{
	std::vector<float> _frame_times;

  public:
	SDL_Window *Window = nullptr;
	SDL_Renderer *Renderer = nullptr;
	SDL_Texture *Runes = nullptr;
	float SampleAverageFPS = 0.0f;

	// (ajout Q1) Liste d'entités pour le pattern Composant
	std::vector<std::unique_ptr<Entity>> Entities;

	// (ajout Q3) je garde la souris + des pointeurs sur mes 3 runes
	MouseInput Mouse;
	Entity* Rune1 = nullptr;
	Entity* Rune2 = nullptr;
	Entity* Rune3 = nullptr;

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

		// (Q1) 

		// ====================== (ajout Q2) Test visuel ======================
		// (ajout Q2) je crée 1 rectangle et 1 sprite juste pour vérifier que mes composants draw marchent.
		// (ajout Q2) les vraies runes + bouton seront faits en Q3.

		// ====================== (ajout Q3) je remplace le test par les vraies entités ======================
		// (ajout Q3) je vide la liste (comme ça j'évite d'avoir les objets test en double)
		Entities.clear();

		// (ajout Q3) je crée 3 runes au milieu : x = 1/4, 2/4, 3/4 ; taille = 128x128
		{
			const float winW = 800.f;
			const float winH = 600.f;
			const float rw = 128.f;
			const float rh = 128.f;

			const float y = (winH * 0.5f) - (rh * 0.5f);

			auto makeRune = [&](float centerX) -> std::unique_ptr<Entity>
			{
				auto e = std::make_unique<Entity>();
				e->AddComponent<TransformComponent>(centerX - rw * 0.5f, y, rw, rh);

				// sprite random
				auto* sprite = e->AddComponent<SpriteRenderComponent>(Runes, RandomRuneSrc());

				// clique => change d'apparence au hasard
				e->AddComponent<ClickableComponent>(&Mouse, [sprite](Entity&)
				{
					sprite->Src = RandomRuneSrc();
				});

				return e;
			};

			auto r1 = makeRune(winW * 0.25f);
			auto r2 = makeRune(winW * 0.50f);
			auto r3 = makeRune(winW * 0.75f);

			Rune1 = r1.get();
			Rune2 = r2.get();
			Rune3 = r3.get();

			Entities.push_back(std::move(r1));
			Entities.push_back(std::move(r2));
			Entities.push_back(std::move(r3));
		}

		// (ajout Q3) je crée le bouton (220x40) à 50 pixels du bas, centré en X
		{
			const float winW = 800.f;
			const float winH = 600.f;

			const float bw = 220.f;
			const float bh = 40.f;
			const float x = (winW * 0.5f) - (bw * 0.5f);
			const float y = (winH - 50.f) - bh;

			auto btn = std::make_unique<Entity>();
			btn->AddComponent<TransformComponent>(x, y, bw, bh);
			btn->AddComponent<RectRenderComponent>(SDL_Color{ 200, 200, 200, 255 });

			// clique bouton => randomize les 3 runes
			btn->AddComponent<ClickableComponent>(&Mouse, [this](Entity&)
			{
				auto randomize = [](Entity* rune)
				{
					if (!rune) return;
					auto* sprite = rune->GetComponent<SpriteRenderComponent>();
					if (sprite) sprite->Src = RandomRuneSrc();
				};

				randomize(Rune1);
				randomize(Rune2);
				randomize(Rune3);
			});

			Entities.push_back(std::move(btn));
		}
	}

	~GameApp ()
	{
		SDL_DestroyTexture (Runes);
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

	// (ajout Q1) UpdateMethod centralisé
	void UpdateAll(float dt)
	{
		for (auto& e : Entities)
			if (e->Alive) e->Update(dt);
	}

	// (ajout Q1) Draw centralisé
	void DrawAll()
	{
		for (auto& e : Entities)
			if (e->Alive) e->Draw(Renderer);
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
			// (ajout Q3) je reset les flags de clic chaque frame
			app->Mouse.BeginFrame();

			SDL_Event event;
			while (SDL_PollEvent (&event) == true)
				{
					if (event.type == SDL_EVENT_QUIT)
						{
							running = false;
						}

					// (ajout Q3) je capture la souris pour pouvoir cliquer sur les runes + bouton
					if (event.type == SDL_EVENT_MOUSE_MOTION)
						{
							app->Mouse.x = event.motion.x;
							app->Mouse.y = event.motion.y;
						}
					if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
						{
							if (event.button.button == SDL_BUTTON_LEFT)
								{
									app->Mouse.leftDown = true;
									app->Mouse.leftPressedThisFrame = true;
								}
						}
					if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
						{
							if (event.button.button == SDL_BUTTON_LEFT)
								{
									app->Mouse.leftDown = false;
									app->Mouse.leftReleasedThisFrame = true;
								}
						}
				}
			const uint64_t freq = SDL_GetPerformanceFrequency ();
			const uint64_t current_time = SDL_GetPerformanceCounter ();
			const float delta_time
				= static_cast<float> (current_time - last_time) / static_cast<float> (freq);
			last_time = current_time;
			app->CalculateFPS (delta_time);

			// (ajout Q1) Update de toutes les entités
			app->UpdateAll(delta_time);

			SDL_SetRenderDrawColor (app->Renderer, 12, 12, 44, 255);
			SDL_RenderClear (app->Renderer);

			SDL_Point win_size = { 0 };
			SDL_GetWindowSize (app->Window, &win_size.x, &win_size.y);

			SDL_SetRenderDrawColor (app->Renderer, 42, 42, 104, 255);
			SDL_RenderLine (app->Renderer, 0.f, 0.f, static_cast<float> (win_size.x),
							static_cast<float> (win_size.y));
			SDL_RenderLine (app->Renderer, 0.f, static_cast<float> (win_size.y),
							static_cast<float> (win_size.x), 0.f);

			// (ajout Q1) Draw de toutes les entités
			app->DrawAll();

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