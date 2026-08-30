import { NextRequest, NextResponse } from "next/server";

export async function GET(request: NextRequest) {
  const searchParams = request.nextUrl.searchParams;
  const ip = searchParams.get("ip") || "192.168.4.1";

  try {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), 1200);

    const res = await fetch(`http://${ip}/api/telemetry`, {
      signal: controller.signal,
      headers: {
        "Cache-Control": "no-cache",
      },
    });
    clearTimeout(timeoutId);

    if (!res.ok) {
      return NextResponse.json({ error: "Device returned non-200" }, { status: 502 });
    }

    const data = await res.json();
    return NextResponse.json(data);
  } catch (err: any) {
    return NextResponse.json(
      { error: "Could not reach device on Wi-Fi", details: err.message },
      { status: 504 }
    );
  }
}
