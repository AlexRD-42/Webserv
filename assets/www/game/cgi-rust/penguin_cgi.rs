use std::env;
use std::fs;
use std::io::{self, Read, Write};
use std::process;
use std::thread;
use std::time::{Duration, SystemTime, UNIX_EPOCH};

const MAX_X: i64 = 500 - 60;
const MAX_Y: i64 = 300 - 80;

const START_X: i64 = 220;
const START_Y: i64 = 110;

const MAX_PLAYERS: usize = 32;
const MAX_BODY: usize = 1024;
const MAX_NAME: usize = 20;
const TTL_MS: u64 = 3000;

const STATE_FILE: &str = "state.txt";
const LOCK_FILE: &str = "state.lock";

struct Player {
    name: String,
    x: i64,
    y: i64,
    facing: char,
}

fn now_ms() -> u64 {
    match SystemTime::now().duration_since(UNIX_EPOCH) {
        Ok(d) => d.as_secs() * 1000 + d.subsec_millis() as u64,
        Err(_) => 0,
    }
}

fn field(body: &str, key: &str) -> Option<String> {
    body.split('&')
        .filter_map(|pair| pair.split_once('='))
        .find(|(k, _)| *k == key)
        .map(|(_, v)| v.to_string())
}

fn cookie(name: &str) -> Option<String> {
    let raw = env::var("HTTP_COOKIE").ok()?;
    raw.split(';')
        .filter_map(|c| c.trim().split_once('='))
        .find(|(k, _)| *k == name)
        .map(|(_, v)| v.trim().to_string())
}

fn clean_name(raw: &str) -> Option<String> {
    let ok = !raw.is_empty()
        && raw.len() <= MAX_NAME
        && raw
            .chars()
            .all(|c| c.is_ascii_alphanumeric() || c == '-' || c == '_');
    if ok {
        Some(raw.to_string())
    } else {
        None
    }
}

fn clean_facing(raw: &str) -> char {
    match raw.chars().next() {
        Some('n') => 'n',
        Some('e') => 'e',
        Some('w') => 'w',
        _ => 's',
    }
}

fn load() -> Vec<(Player, u64)> {
    let text = match fs::read_to_string(STATE_FILE) {
        Ok(t) => t,
        Err(_) => return Vec::new(),
    };

    let mut players = Vec::new();
    for line in text.lines() {
        let f: Vec<&str> = line.split_whitespace().collect();
        if f.len() != 5 {
            continue;
        }
        let name = match clean_name(f[0]) {
            Some(n) => n,
            None => continue,
        };
        let player = Player {
            name,
            x: f[1].parse().unwrap_or(START_X).clamp(0, MAX_X),
            y: f[2].parse().unwrap_or(START_Y).clamp(0, MAX_Y),
            facing: clean_facing(f[3]),
        };
        players.push((player, f[4].parse().unwrap_or(0)));
        if players.len() >= MAX_PLAYERS {
            break;
        }
    }
    players
}

fn save(players: &[(Player, u64)]) {
    let mut text = String::new();
    for (p, seen) in players {
        text.push_str(&format!(
            "{} {} {} {} {}\n",
            p.name, p.x, p.y, p.facing, seen
        ));
    }

    let tmp = format!("{}.{}.tmp", STATE_FILE, process::id());
    if fs::write(&tmp, text).is_ok() {
        let _ = fs::rename(&tmp, STATE_FILE);
    }
}

struct Lock;

impl Drop for Lock {
    fn drop(&mut self) {
        let _ = fs::remove_file(LOCK_FILE);
    }
}

fn lock() -> Option<Lock> {
    for _ in 0..400 {
        if fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(LOCK_FILE)
            .is_ok()
        {
            return Some(Lock);
        }
        if let Ok(meta) = fs::metadata(LOCK_FILE) {
            if let Ok(modified) = meta.modified() {
                if modified
                    .elapsed()
                    .map(|d| d.as_secs() >= 5)
                    .unwrap_or(false)
                {
                    let _ = fs::remove_file(LOCK_FILE);
                    continue;
                }
            }
        }
        thread::sleep(Duration::from_millis(5));
    }
    None
}

fn read_body() -> String {
    let declared: usize = env::var("CONTENT_LENGTH")
        .ok()
        .and_then(|s| s.trim().parse().ok())
        .unwrap_or(0);

    let mut stdin = io::stdin();
    let mut buf = vec![0u8; declared.min(MAX_BODY)];
    if stdin.read_exact(&mut buf).is_err() {
        buf.clear();
    }
    if declared > MAX_BODY {
        let extra = (declared - MAX_BODY) as u64;
        let _ = io::copy(&mut stdin.take(extra), &mut io::sink());
    }

    String::from_utf8_lossy(&buf).into_owned()
}

fn saved_position() -> (i64, i64, char) {
    if let Some(value) = cookie("cp_pos") {
        let f: Vec<&str> = value.split('.').collect();
        if f.len() == 3 {
            return (
                f[0].parse().unwrap_or(START_X).clamp(0, MAX_X),
                f[1].parse().unwrap_or(START_Y).clamp(0, MAX_Y),
                clean_facing(f[2]),
            );
        }
    }
    (START_X, START_Y, 's')
}

fn respond(status: &str, extra_header: &str, body: &str) {
    print!(
        "Status: {}\r\n\
         Content-Type: application/json\r\n\
         Content-Length: {}\r\n\
         Cache-Control: no-store\r\n\
         {}\r\n\
         {}",
        status,
        body.len(),
        extra_header,
        body
    );
    let _ = io::stdout().flush();
}

fn error(status: &str, message: &str) {
    respond(status, "", &format!("{{\"error\":\"{}\"}}", message));
}

fn room_json(players: &[(Player, u64)], me: &str) -> String {
    let mut out = format!("{{\"you\":\"{}\",\"players\":[", me);
    for (i, (p, _)) in players.iter().enumerate() {
        if i > 0 {
            out.push(',');
        }
        out.push_str(&format!(
            "{{\"username\":\"{}\",\"x\":{},\"y\":{},\"facing\":\"{}\"}}",
            p.name, p.x, p.y, p.facing
        ));
    }
    out.push_str("]}");
    out
}

fn main() {
    let method = env::var("REQUEST_METHOD").unwrap_or_default();

    let me = match cookie("cp_session").as_deref().and_then(clean_name) {
        Some(name) => name,
        None => return error("401 Unauthorized", "no session"),
    };

    let body = if method == "POST" {
        read_body()
    } else {
        String::new()
    };

    let _guard = match lock() {
        Some(g) => g,
        None => return error("503 Service Unavailable", "room busy"),
    };

    let now = now_ms();
    let mut players = load();

    players.retain(|(_, seen)| now.saturating_sub(*seen) < TTL_MS);

    if field(&body, "leave").is_some() {
        players.retain(|(p, _)| p.name != me);
        save(&players);
        return respond("200 OK", "", "{\"left\":true}");
    }

    let index = match players.iter().position(|(p, _)| p.name == me) {
        Some(i) => i,
        None => {
            if players.len() >= MAX_PLAYERS {
                return error("503 Service Unavailable", "room full");
            }
            let (x, y, facing) = saved_position();
            players.push((
                Player {
                    name: me.clone(),
                    x,
                    y,
                    facing,
                },
                now,
            ));
            players.len() - 1
        }
    };

    {
        let (p, seen) = &mut players[index];
        if let Some(v) = field(&body, "x") {
            p.x = v.parse().unwrap_or(p.x).clamp(0, MAX_X);
        }
        if let Some(v) = field(&body, "y") {
            p.y = v.parse().unwrap_or(p.y).clamp(0, MAX_Y);
        }
        if let Some(v) = field(&body, "facing") {
            p.facing = clean_facing(&v);
        }
        *seen = now;
    }

    save(&players);

    let (p, _) = &players[index];
    let set_cookie = format!(
        "Set-Cookie: cp_pos={}.{}.{}; Path=/; Max-Age=86400; SameSite=Lax\r\n",
        p.x, p.y, p.facing
    );

    respond("200 OK", &set_cookie, &room_json(&players, &me));
}
