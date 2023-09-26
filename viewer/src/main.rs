use std::process::Command;

use ori::prelude::*;

pub struct Gaussian {
    pub size: u32,
    pub sigma: f32,
}

impl Default for Gaussian {
    fn default() -> Self {
        Self {
            size: 11,
            sigma: 1.0,
        }
    }
}

pub struct Laplacian {
    pub size: u32,
}

impl Default for Laplacian {
    fn default() -> Self {
        Self { size: 11 }
    }
}

pub enum PassKind {
    Gaussian(Gaussian),
    Laplacian(Laplacian),
}

pub struct Pass {
    pub kind: PassKind,
    pub output: Image,
    pub enabled: bool,
}

impl Pass {
    pub fn gaussian() -> Self {
        Self {
            kind: PassKind::Gaussian(Gaussian::default()),
            output: Image::default(),
            enabled: true,
        }
    }

    pub fn laplacian() -> Self {
        Self {
            kind: PassKind::Laplacian(Laplacian::default()),
            output: Image::default(),
            enabled: true,
        }
    }
}

#[derive(Default)]
pub struct Viewer {
    pub passes: Vec<Pass>,
    pub selected: Option<usize>,
}

impl Viewer {
    const DIST_DIR: &'static str = "dist";
    const PASS_DIR: &'static str = "dist/passes";

    pub fn new() -> Self {
        Self {
            passes: vec![Pass::gaussian(), Pass::laplacian()],
            selected: None,
        }
    }

    fn generate_arguments(&mut self) -> Vec<String> {
        let mut args = Vec::new();

        args.push(String::from("--input samples/easy/1EASY.bmp"));
        args.push(format!("--output {}/output.bmp", Self::DIST_DIR));
        args.push(format!("--pass-dir {}", Self::PASS_DIR));

        for pass in self.passes.iter() {
            if !pass.enabled {
                continue;
            }

            match pass.kind {
                PassKind::Gaussian(ref gaussian) => {
                    args.push(String::from("--kernel 1"));
                    args.push(format!("--kernel-size {}", gaussian.size));
                    args.push(format!("--kernel-args {}", gaussian.sigma));
                }
                PassKind::Laplacian(ref laplacian) => {
                    args.push(String::from("--kernel 2"));
                    args.push(format!("--kernel-size {}", laplacian.size));
                }
            }
        }

        args
    }

    fn run(&mut self) {
        let args = self.generate_arguments();

        let bin = if cfg!(target_os = "windows") {
            "dist/windows/cells.exe"
        } else {
            "dist/linux/cells.exe"
        };

        let command = Command::new(bin).args(args).output();

        match command {
            Ok(_) => {
                for (i, pass) in self.passes.iter_mut().enumerate() {
                    if !pass.enabled {
                        continue;
                    }

                    let path = format!("{}/kernel_{}.bmp", Self::PASS_DIR, i);
                    let data = std::fs::read(path).unwrap();
                    pass.output = Image::load_data(data);
                }
            }
            Err(err) => {
                error!("Failed to run cells: {}", err);
            }
        }
    }

    fn run_button(&mut self) -> impl View<Self> {
        let run = button(text("Run")).padding(rem(0.3));

        on_press(run, |_, data: &mut Self| {
            data.run();
        })
    }

    fn add_gaussian_button(&mut self) -> impl View<Self> {
        let add = button(text("Add Gaussian")).padding(rem(0.3));

        on_press(add, |_, data: &mut Self| {
            data.passes.push(Pass::gaussian());
        })
    }

    fn add_laplacian_button(&mut self) -> impl View<Self> {
        let add = button(text("Add Laplacian")).padding(rem(0.3));

        on_press(add, |_, data: &mut Self| {
            data.passes.push(Pass::laplacian());
        })
    }

    fn pass(&mut self, index: usize) -> impl View<Self> {
        let pass = &mut self.passes[index];

        let enabled = on_press(
            checkbox(pass.enabled).border_radius(pt(12.0)),
            move |_, data: &mut Self| {
                data.passes[index].enabled = !data.passes[index].enabled;
            },
        );

        let remove = on_press(
            button(fa::icon("xmark"))
                .padding(rem(0.2))
                .fancy(pt(4.0))
                .color(hsl(0.0, 0.5, 0.5)),
            move |_, data: &mut Self| {
                data.passes.remove(index);
                data.selected = None;
            },
        );

        let pass = match pass.kind {
            PassKind::Gaussian(ref gaussian) => {
                let size = on_event_after(
                    text(format!("Size: {}", gaussian.size)),
                    move |cx, data: &mut Self, event| {
                        if !cx.is_hot() {
                            return;
                        }

                        let gaussian = match &mut data.passes[index].kind {
                            PassKind::Gaussian(gaussian) => gaussian,
                            _ => unreachable!(),
                        };

                        if let Some(pointer) = event.get::<PointerEvent>() {
                            if pointer.scroll.y > 0.0 {
                                gaussian.size += 2;
                                cx.request_rebuild();
                            } else if pointer.scroll.y < 0.0 && gaussian.size > 3 {
                                gaussian.size -= 2;
                                cx.request_rebuild();
                            }
                        }
                    },
                );

                let sigma = on_event_after(
                    text(format!("Sigma: {:.2}", gaussian.sigma)),
                    move |cx, data: &mut Self, event| {
                        if !cx.is_hot() {
                            return;
                        }

                        let gaussian = match &mut data.passes[index].kind {
                            PassKind::Gaussian(gaussian) => gaussian,
                            _ => unreachable!(),
                        };

                        if let Some(pointer) = event.get::<PointerEvent>() {
                            if pointer.scroll.y != 0.0 {
                                gaussian.sigma += pointer.scroll.y * 0.05;
                                cx.request_rebuild();
                            }
                        }
                    },
                );

                let data = flex(1.0, width(FILL, vstack![text("Gaussian"), size, sigma]));

                let control = vstack![enabled, remove].gap(rem(0.5));

                any(hstack![control, data].gap(rem(0.5)))
            }
            PassKind::Laplacian(ref laplacian) => {
                let size = on_event_after(
                    text(format!("Size: {}", laplacian.size)),
                    move |cx, data: &mut Self, event| {
                        if !cx.is_hot() {
                            return;
                        }

                        let laplacian = match &mut data.passes[index].kind {
                            PassKind::Laplacian(laplacian) => laplacian,
                            _ => unreachable!(),
                        };

                        if let Some(pointer) = event.get::<PointerEvent>() {
                            if pointer.scroll.y > 0.0 {
                                laplacian.size += 2;
                                cx.request_rebuild();
                            } else if pointer.scroll.y < 0.0 && laplacian.size > 3 {
                                laplacian.size -= 2;
                                cx.request_rebuild();
                            }
                        }
                    },
                );

                let data = flex(1.0, width(FILL, vstack![text("Laplacian"), size]));

                let control = vstack![enabled, remove].gap(rem(0.5));

                any(hstack![control, data].gap(rem(0.5)))
            }
        };

        let color = if self.selected == Some(index) {
            style(Palette::ACCENT)
        } else {
            style(Palette::BACKGROUND)
        };

        container(pad(rem(0.5), pass))
            .background(color)
            .border_width(pt(2.0))
            .border_color(style(Palette::SECONDARY_DARKER))
            .border_radius(rem(0.5))
    }

    fn pass_list(&mut self) -> impl View<Self> {
        let mut views = Vec::new();

        for i in 0..self.passes.len() {
            let pass = on_click(self.pass(i), move |_, data: &mut Self| {
                data.selected = Some(i);
                info!("Selected pass {}", i);
            });

            views.push(width(rem(10.0), pass));
        }

        vstack![for views].gap(rem(0.2))
    }

    fn output(&mut self) -> impl View<Self> {
        match self.selected {
            Some(index) => self.passes[index].output.clone(),
            None => Image::default(),
        }
    }

    pub fn ui(&mut self) -> impl View<Self> {
        let right_panel = vstack![
            self.pass_list(),
            self.add_gaussian_button(),
            self.add_laplacian_button(),
            self.run_button(),
        ]
        .align_items(Align::Stretch)
        .gap(rem(0.2));

        let right_pad = pad(rem(0.5), right_panel);

        size(
            FILL,
            hstack![flex(1.0, size(FILL, self.output())), top(right_pad)],
        )
    }
}

fn main() {
    App::new(Viewer::ui, Viewer::new())
        .title("Cell Detector Fast")
        .msaa(true)
        .run();
}
