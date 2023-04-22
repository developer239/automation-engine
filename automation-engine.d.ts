// Interface definitions 0.1
// ---------------------

// Basic interfaces
declare interface ISize {
  width: number
  height: number
}

declare interface IPosition {
  x: number
  y: number
}

declare interface IColor {
  b: number
  g: number
  r: number
}

// Component interfaces
declare interface IBoundingBoxComponent {
  position: IPosition
  size: ISize
  color: IColor
  thickness: number
}

declare interface ITextLabelComponent {
  text: string
  position: IPosition
  color: IColor
}

declare interface IEditableComponent {
  isEditable: boolean
}

// Detection components and operations interfaces
declare interface IDetectContoursComponent {
  id: string
  minArea: ISize
  maxArea?: ISize
  bboxColor: IColor
  bboxThickness?: number
  shouldRenderPreview?: boolean
}

declare interface IDetectTextComponent {
  id: string
  bboxColor: IColor
  bboxThickness?: number
  shouldRenderPreview?: boolean
}

declare interface IDetectColorsOperation {
  lowerBound: IColor
  upperBound: IColor
}

declare interface CropOperation {
  position: IPosition
  size: ISize
}

declare interface IDetectObjectsComponent {
  id: string
  confidenceThreshold: number
  nonMaximumSuppressionThreshold: number
  pathToModel: string
  pathToClasses: string
}

declare interface IInstanceSegmentationComponent {
  id: string
  confidenceThreshold: number
  nonMaximumSuppressionThreshold: number
  pathToModel: string
  pathToClasses: string
}

declare interface MorphologyOperation {
  size: ISize
}

// Main interface
declare interface IMain {
  onUpdate: () => void
  screen: {
    width: number
    height: number
    x: number
    y: number
  }
  entities?: {
    tag: string
    groups?: string[]
    isEditable?: boolean
    components: {
      boundingBox?: MakePartial<IBoundingBoxComponent, 'color' | 'thickness'>
      textLabel?: ITextLabelComponent
    }
  }[]
}

// Class and function declarations
// --------------------------------

declare class Entity {
  getId(): number

  toString(): string

  setTag(tag: string): void

  getTag(): string

  addGroup(group: string): void

  getGroups(): string[]

  removeGroup(group: string): void

  removeGroups(): void

  getComponentBoundingBox(): IBoundingBoxComponent

  addComponentBoundingBox(component: IBoundingBoxComponent): void

  getComponentEditable(): IEditableComponent

  addComponentEditable(): void

  getComponentTextLabel(): ITextLabelComponent

  addComponentTextLabel(component: ITextLabelComponent): void

  addComponentDetection(): void

  addComponentDetectionMorphologyOperation(
    type: 'close' | 'open' | 'erode' | 'dilate',
    operation: MorphologyOperation
  ): void

  addComponentDetectionColorsOperation(operation: IDetectColorsOperation): void

  addComponentDetectionCropOperation(operation: CropOperation): void

  addComponentDetectContours(component: IDetectContoursComponent): void

  addComponentDetectText(component: IDetectTextComponent): void

  addComponentDetectObjects(component: IDetectObjectsComponent): void

  addComponentInstanceSegmentation(
    component: IInstanceSegmentationComponent
  ): void
}

declare class Keyboard {
  static Instance(): Keyboard

  type(text: string): void

  enter(): void

  arrowUp(): void

  arrowDown(): void

  arrowLeft(): void

  arrowRight(): void

  escape(): void

  space(): void
}

declare class Mouse {
  static Instance(): Mouse

  move(x: number, y: number): void
}

declare class Bus {
  static Instance(): Bus

  emitMessageEvent(text: string): void
}

declare class Registry {
  static Instance(): Registry

  createEntity(): Entity

  killEntity(entity: Entity): void

  getEntityByTag(tag: string): Entity

  getEntitiesByGroup(group: string): Entity[]
}

// NOTE: "std::vector<TType>" is printed as "Table []"
declare function printTable(this: void, table: any): void

declare function getTicks(): number

declare function sortByX(
  this: void,
  entities: Entity[],
  descending: boolean
): void

declare function checkCollision(this: void, a: Entity, b: Entity): boolean

// Utility type
type MakePartial<T, K extends keyof T> = Omit<T, K> & Partial<Pick<T, K>>

// Global variable
declare var main: IMain
