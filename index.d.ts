export interface Point {
  x: number;
  y: number;
}

export interface Rectangle {
  left: number;
  top: number;
  right: number;
  bottom: number;
}

export interface Window {
  handle: number;
  title: string;
}

export function getOpenedWindows(): Window[];
export function windowExists(handle: number): boolean;
export function showWindow(handle: number): void;
export function getWindowRectangle(handle: number): Rectangle;
export function getCursorPosition(): Point;
export function setCursorPosition(x: number, y: number): void;
export function leftClickDown(): void;
export function leftClickUp(): void;
export function enterKeyDown(): void;
export function enterKeyUp(): void;
